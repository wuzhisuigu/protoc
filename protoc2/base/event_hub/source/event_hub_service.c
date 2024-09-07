#include <ohos_init.h>
#include "hiview_log.h"
#include "iunknown.h"
#include "message.h"
#include "ohos_types.h"
#include "samgr_common.h"
#include "service.h"
#include "memory_adapter.h"
#include "thread_adapter.h"
#include "queue_adapter.h"
#include "time_adapter.h"
#include "samgr_lite.h"
#include "timer_adapter.h"
#include "utils_list.h"
#include "event_hub_service.h"

#include <memory_adapter.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "dfx_log.h"

#include "event_hub_priv.h"

#define DFX_LOG_MODULE_ID OHOS_MODULE_EVENT_HUB

// #define LOGD(...) HILOG_DEBUG(HILOG_MODULE_APP, __VA_ARGS__)
// #define LOGI(...) HILOG_INFO(HILOG_MODULE_APP, __VA_ARGS__)
// #define LOGE(...) HILOG_ERROR(HILOG_MODULE_APP, __VA_ARGS__)

static const char *GetName(Service *service);
static BOOL Initialize(Service *service, Identity identity);
static TaskConfig GetTaskConfig(Service *service);
static BOOL MessageHandle(Service *service, Request *msg);
EventHubService event_hub_service = {
    .GetName = GetName,
    .Initialize = Initialize,
    .MessageHandle = MessageHandle,
    .GetTaskConfig = GetTaskConfig,
};

static void EventHub_FireEvents(EventHubService *eventHub, pHandle senderHandle, uint32 msgId)
{
    EventHubSenderNode *item = NULL, *next = NULL, *sender = NULL;
    EventHubReceiverNode *r_item = NULL, *r_next = NULL;
    if (UtilsListEmpty(&eventHub->receiverList)) {
        return;
    } else {
        UTILS_DL_LIST fired_list;
        UtilsListInit(&fired_list);
        // find sender handle
        UTILS_DL_LIST_FOR_EACH_ENTRY_SAFE(item, next, &eventHub->senderList, EventHubSenderNode,
                                          link) {
            if (item->handle == senderHandle) {
                sender = item;
                break;
            }
        }

        if (!sender) {
            return;
        }
        LOGD("event_hub fire event sender sid = %d\n", sender->sendIdentity.serviceId);
        // find receiver handle
        UTILS_DL_LIST_FOR_EACH_ENTRY_SAFE(r_item, r_next, &eventHub->receiverList,
                                          EventHubReceiverNode, link) {
            if (!memcmp(&item->sendIdentity, &r_item->sendIdentity, sizeof(Identity))) {
                Request request = { 0 };
                request.msgId = r_item->msgId;
                request.msgValue = msgId;
                LOGD("event_hub fire event to sid = %d\n", r_item->recvIdentity.serviceId);
                SAMGR_SendRequest(&r_item->recvIdentity, &request, NULL);
            }
        }
    }
}

static void EventHub_UpdateReceiverNode(EventHubService *eventHub, EventHubReceiverNode *node,
                                        BOOL *nodeRefered)
{
    // BOOL found = FALSE;
    EventHubReceiverNode *item, *next;

    LOGD("add event_hub receiver sid=%d, sender sid = %d\n", node->recvIdentity.serviceId,
         node->sendIdentity.serviceId);
    UTILS_DL_LIST_FOR_EACH_ENTRY_SAFE(item, next, &eventHub->receiverList, EventHubReceiverNode,
                                      link) {
        if (!memcmp(&item->recvIdentity, &node->recvIdentity, sizeof(Identity)) &&
            !memcmp(&item->sendIdentity, &node->sendIdentity, sizeof(Identity))) {
            // found, delete the old one
            // found = TRUE;
            UtilsListDelete(&item->link);
            // remove this node
            MEM_Free(item);
            break;
        }
    }

    // new node
    if (node->sendIdentity.queueId != NULL) {
        UtilsListInit(&node->link);
        UtilsListAdd(&eventHub->receiverList, &node->link);
        /* this node is refered, don't release it, it will be released in this module when deleted */
        *nodeRefered = TRUE;
    } else {
        *nodeRefered = FALSE;
    }
}

static void EventHub_UpdateSenderNode(EventHubService *eventHub, EventHubSenderNode *node,
                                      BOOL *nodeRefered)
{
    // BOOL found = FALSE;
    EventHubSenderNode *item, *next;

    LOGD("add event_hub sender sid = %d\n", node->sendIdentity.serviceId);
    UTILS_DL_LIST_FOR_EACH_ENTRY_SAFE(item, next, &eventHub->senderList, EventHubSenderNode,
                                      link) {
        if (item->handle == node->handle) {
            // found, delete the old one
            // found = TRUE;
            UtilsListDelete(&item->link);
            // remove this node
            MEM_Free(item);
            break;
        }
    }

    // new node
    if (node->sendIdentity.queueId != NULL) {
        UtilsListInit(&node->link);
        UtilsListAdd(&eventHub->senderList, &node->link);
        /* this node is refered, don't release it, it will be released in this module when deleted */
        *nodeRefered = TRUE;
    } else {
        *nodeRefered = FALSE;
    }
}

static const char *GetName(Service *service)
{
    (void)service;
    return EVENT_HUB_SERVICE;
}
static BOOL Initialize(Service *service, Identity identity)
{
    EventHubService *eventHub = (EventHubService *)service;
    eventHub->identity = identity;
    UtilsListInit(&eventHub->senderList);
    UtilsListInit(&eventHub->receiverList);

    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *msg)
{
    BOOL msgDataRefered = FALSE;
    // LOGI("[EventHub] msgId<%d>\n",  msg->msgId);
    EventHubService *eventHub = (EventHubService *)service;
    switch (msg->msgId) {
    case EVENT_HUB_MSG_SEND_EVENT:
        EventHub_FireEvents(eventHub, (pHandle)msg->data, msg->msgValue);
        break;
    case EVENT_HUB_MSG_ADD_SENDER:
        EventHub_UpdateSenderNode(eventHub, (EventHubSenderNode *)msg->data, &msgDataRefered);
        if (msgDataRefered) {
            // tell the framework not to free it
            msg->len = 0;
        }
        break;
    case EVENT_HUB_MSG_ADD_RECEIVER:
        EventHub_UpdateReceiverNode(eventHub, (EventHubReceiverNode *)msg->data,
                                    &msgDataRefered);
        if (msgDataRefered) {
            // tell the framework not to free it
            msg->len = 0;
        }
        break;
    }

    (void)eventHub;
    return FALSE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    // SHARED_TASK use config in DEFAULT_TASK_CFG, don't use the setting here
    TaskConfig config = { LEVEL_HIGH, PRI_BELOW_NORMAL, 0, 0, SHARED_TASK };
    return config;
}

/* module entry function can not be static, it is forced to be linked into the target executable
  by linker option -u */
void EventHubInit(void)
{
    EventHubService *eventHub = &event_hub_service;

    SAMGR_GetInstance()->RegisterService((Service *)eventHub);
}
CORE_INIT(EventHubInit);