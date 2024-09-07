#include "message.h"
#include "ohos_types.h"
#include "samgr_timer.h"
#include "samgr_lite.h"
#include "memory_adapter.h"
#include "utils_list.h"

#include "samgr_timer_priv.h"

void SamgrTimer_UpdateTime(pHandle handle,  Identity *targetId, uint32 msgId, uint32 interval, int32 repeatCnt)
{
    SamgrTimerService *timerService = (SamgrTimerService*)handle;
    if (!timerService) {
        return;
    }

    SamgrTimerParams *timerParams = MEM_Malloc(sizeof(SamgrTimerParams));
    timerParams->identity = *targetId;
    timerParams->msgId = msgId;
    timerParams->interval = interval;
	timerParams->repeatCnt = repeatCnt;

    Request request = {0};
    request.msgId = SAMGR_TIMER_MSG_UPDATE_TIMER;
    request.data = timerParams;
    request.len = sizeof(SamgrTimerParams);
    SAMGR_SendRequest(&timerService->identity, &request, NULL);
}

pHandle SamgrTimer_GetInstance(void)
{
    if (samgr_timer_service.identity.queueId) {
        return &samgr_timer_service;
    } else {
        return NULL;
    }
}
