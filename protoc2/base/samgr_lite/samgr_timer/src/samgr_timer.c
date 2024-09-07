#include <ohos_init.h>
#include "hiview_log.h"
#include "iunknown.h"
#include "message.h"
#include "samgr_common.h"
#include "service.h"
#include "memory_adapter.h"
#include "thread_adapter.h"
#include "queue_adapter.h"
#include "time_adapter.h"
#include "samgr_lite.h"
#include "timer_adapter.h"
#include "utils_list.h"
#include "samgr_timer.h"

#include <memory_adapter.h>
#include <stdint.h>
#include <string.h>
#include "log.h"

#include "samgr_timer_priv.h"

#define LOGI(...) HILOG_INFO(HILOG_MODULE_APP, __VA_ARGS__)
#define LOGE(...) HILOG_ERROR(HILOG_MODULE_APP, __VA_ARGS__)

#if defined(__WIN64)
#define USE_SAMGR_TIMER_THREAD
#endif

#define DONT_WAIT 0
#define WAIT_FOREVER (-1)

// use 24bit time only, to avoid wrap arround in (now + interval)
#define SAMGR_TIMER_TIME_MAX 0x0FFFFFFF

typedef struct SamgrTimerNode
{
    UTILS_DL_LIST link;
    SamgrTimerParams params;
    uint32 expire_time;
} SamgrTimerNode;

static const char *GetName(Service *service);
static BOOL Initialize(Service *service, Identity identity);
static TaskConfig GetTaskConfig(Service *service);
static BOOL MessageHandle(Service *service, Request *msg);
SamgrTimerService samgr_timer_service = {
    .GetName = GetName,
    .Initialize = Initialize,
    .MessageHandle = MessageHandle,
    .GetTaskConfig = GetTaskConfig,
};

static void SamgrTimer_InsertNodeOrdered(SamgrTimerService *samgrTimer, SamgrTimerNode *node)
{
    BOOL bInserted = FALSE;
    SamgrTimerNode *item = NULL, *next = NULL;
    if (UtilsListEmpty(&samgrTimer->timerList))
    {
        UtilsListAdd(&samgrTimer->timerList, &node->link);
    }
    else
    {
        UTILS_DL_LIST_FOR_EACH_ENTRY_SAFE(item, next, &samgrTimer->timerList, SamgrTimerNode,
                                          link)
        {
            if (item->expire_time > node->expire_time)
            {
                UtilsListInsert(&item->link, &node->link);
                bInserted = TRUE;
                break;
            }
        }

        if (!bInserted)
        {
            // add to tail
            UtilsListAdd(&samgrTimer->timerList, &node->link);
        }
    }
}

static void SamgrTimer_FireTimers(SamgrTimerService *samgrTimer)
{
    SamgrTimerNode *item = NULL, *next = NULL;
    UTILS_DL_LIST *dl_item = NULL, *dl_next = NULL;
    uint32 now = (uint32)SAMGR_GetProcessTime() & SAMGR_TIMER_TIME_MAX;
    uint32 wrap = FALSE;
    // update tick_cnt_rd for message queue flow ctrl
    samgrTimer->tickCntRd++;

    if (UtilsListEmpty(&samgrTimer->timerList))
    {
        samgrTimer->lastTick = now;
        return;
    }
    else
    {
        UTILS_DL_LIST fired_list;
        UtilsListInit(&fired_list);
        if (now < samgrTimer->lastTick)
        {
            // wrap arrownd
            wrap = TRUE;
        }

        samgrTimer->lastTick = now;

        UTILS_DL_LIST_FOR_EACH_ENTRY_SAFE(item, next, &samgrTimer->timerList, SamgrTimerNode,
                                          link)
        {
            if (wrap)
            {
                if (item->expire_time > SAMGR_TIMER_TIME_MAX)
                {
                    item->expire_time = item->expire_time - SAMGR_TIMER_TIME_MAX;
                }
                else
                {
                    item->expire_time = 0;
                }
            }

            if (item->expire_time <= now)
            {
                Request request = {0};
                request.msgId = item->params.msgId;
                request.msgValue = now;
                SAMGR_SendRequest(&item->params.identity, &request, NULL);

                UtilsListDelete(&item->link);
                if (item->params.repeatCnt != SAMGR_TIMER_REPEAT_INFINITE)
                {
                    item->params.repeatCnt--;
                }

                if (item->params.repeatCnt == 0)
                {
                    // free it
                    MEM_Free(item);
                }
                else
                {
                    UtilsListInit(&item->link);
                    item->expire_time += item->params.interval;
                    if (item->expire_time < now)
                    {
                        // too late, catch up
                        item->expire_time = now + item->params.interval;
                    }
                    // add it back to the list, to be fired next time
                    // SamgrTimer_InsertNodeOrdered(samgrTimer, item);
                    UtilsListAdd(&fired_list, &item->link);
                }
            }
        }

        UTILS_DL_LIST_FOR_EACH_SAFE(dl_item, dl_next, &fired_list)
        {
            UtilsListDelete(dl_item);
            UtilsListInit(dl_item);
            item = UTILS_DL_LIST_ENTRY(dl_item, SamgrTimerNode, link);
            SamgrTimer_InsertNodeOrdered(samgrTimer, item);
        }
    }
}

static void SamgrTimer_UpdateTimer(SamgrTimerService *samgrTimer, SamgrTimerParams *params)
{
    BOOL found = FALSE;
    SamgrTimerNode *item, *next;
    uint32 now = (uint32)SAMGR_GetProcessTime() & SAMGR_TIMER_TIME_MAX;
    UTILS_DL_LIST update_list;
    UTILS_DL_LIST *dl_item;
    UTILS_DL_LIST *dl_next;
    UtilsListInit(&update_list);

    HILOG_INFO(HILOG_MODULE_SAMGR, "Timer update from sid %d, interval %d\n",
               params->identity.serviceId, params->interval);
    UTILS_DL_LIST_FOR_EACH_ENTRY_SAFE(item, next, &samgrTimer->timerList, SamgrTimerNode,
                                      link)
    {
        if (!memcmp(&item->params.identity, &params->identity, sizeof(params->identity)) &&
            (item->params.msgId == params->msgId))
        {
            // found
            found = TRUE;
            UtilsListDelete(&item->link);
            if (params->interval && (params->repeatCnt != 0))
            {
                // update
                item->params = *params;
                item->expire_time = now + item->params.interval;
                UtilsListInit(&item->link);
                // add it back to the list, to be fired next time
                UtilsListAdd(&update_list, &item->link);
            }
            else
            {
                // remove this timer
                MEM_Free(item);
            }
            break;
        }
    }

    // new timer
    if (!found && params->interval > 0 && (params->repeatCnt != 0))
    {
        item = (SamgrTimerNode *)MEM_Malloc(sizeof(SamgrTimerNode));
        item->params = *params;
        item->expire_time = now + item->params.interval;
        UtilsListInit(&item->link);
        SamgrTimer_InsertNodeOrdered(samgrTimer, item);
    }

    UTILS_DL_LIST_FOR_EACH_SAFE(dl_item, dl_next, &update_list)
    {
        UtilsListDelete(dl_item);
        UtilsListInit(dl_item);
        item = UTILS_DL_LIST_ENTRY(dl_item, SamgrTimerNode, link);
        SamgrTimer_InsertNodeOrdered(samgrTimer, item);
    }
}

static void samgrTimerTickReq(SamgrTimerService *samgrTimer)
{
    uint8 tick_cnt_rd = 0;
    uint8 tick_cnt_wr = 0;
    uint8 tick_cnt_diff = 0;

    tick_cnt_rd = samgrTimer->tickCntRd;
    tick_cnt_wr = samgrTimer->tickCntWr;
    if (tick_cnt_wr >= tick_cnt_rd)
    {
        tick_cnt_diff = tick_cnt_wr - tick_cnt_rd;
    }
    else
    {
        tick_cnt_diff = 0xFF - tick_cnt_rd + tick_cnt_wr;
    }
    // flow ctrl for message queue, don't put too many request to the message queue
    // the timer service task maybe blocked for a long time and can not process tick message in time
    if (tick_cnt_diff < 2)
    {
        Request request = {0};
        request.msgId = SAMGR_TIMER_MSG_TICK;
        SAMGR_SendRequest(&samgrTimer->identity, &request, NULL);
        samgrTimer->tickCntWr++;
    }
}

#ifdef USE_SAMGR_TIMER_THREAD
static void *SamgrTimer_Task(void *arg)
{
    SamgrTimerService *samgrTimer = (SamgrTimerService *)arg;

    while (1)
    {
        THREAD_Sleep(10);
        samgrTimerTickReq(samgrTimer);
    }
    return NULL;
}
#else
static void timer_tick(TimerId id)
{
    SamgrTimerService *samgrTimer = (SamgrTimerService *)TIMER_GetUserData(id);
    samgrTimerTick(samgrTimer);
}
#endif

static const char *GetName(Service *service)
{
    (void)service;
    return SAMGR_TIMER_SERVICE;
}
static BOOL Initialize(Service *service, Identity identity)
{
    SamgrTimerService *samgrTimer = (SamgrTimerService *)service;
    samgrTimer->identity = identity;

#ifdef USE_SAMGR_TIMER_THREAD
    ThreadAttr attr = {0};
    attr.name = "samgrTimer";
    attr.priority = PRI_BELOW_NORMAL;
    attr.stackSize = 0x800;
    THREAD_Create(SamgrTimer_Task, samgrTimer, &attr);
#else
    TimerAttr attr = {0};
    TimerId tickTimer;
    attr.name = "samgrTimer";
    attr.autoReload = TRUE;
    attr.userData = samgrTimer;
    tickTimer = TIMER_Create(timer_tick, 10, &attr);
    TIMER_Start(tickTimer);
#endif

    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *msg)
{
    // LOGI("[SamgrTimer] msgId<%d>\n",  msg->msgId);
    SamgrTimerService *samgrTimer = (SamgrTimerService *)service;
    switch (msg->msgId)
    {
    case SAMGR_TIMER_MSG_TICK:
        SamgrTimer_FireTimers(samgrTimer);
        break;
    case SAMGR_TIMER_MSG_UPDATE_TIMER:
        SamgrTimer_UpdateTimer(samgrTimer, (SamgrTimerParams *)msg->data);
        break;
    }

    (void)samgrTimer;
    return FALSE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    // SHARED_TASK use config in DEFAULT_TASK_CFG, don't use the setting here
    TaskConfig config = {LEVEL_HIGH, PRI_BELOW_NORMAL, 0, 0, SHARED_TASK};
    return config;
}

/* module entry function can not be static, it is forced to be linked into the target executable
  by linker option -u */
void SamgrTimerInit(void)
{
    UtilsListInit(&samgr_timer_service.timerList);

    SAMGR_GetInstance()->RegisterService((Service *)&samgr_timer_service);
}
CORE_INIT(SamgrTimerInit);
