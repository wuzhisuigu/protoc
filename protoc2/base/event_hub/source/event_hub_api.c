#include "message.h"
#include "ohos_types.h"
#include "event_hub_service.h"
#include "samgr_lite.h"
#include "memory_adapter.h"
#include "utils_list.h"

#include "dfx_log.h"
#include "event_hub_priv.h"

#define DFX_LOG_MODULE_ID OHOS_MODULE_EVENT_HUB

// #define LOGD(...) HILOG_DEBUG(HILOG_MODULE_APP, __VA_ARGS__)

static Identity g_evtHubId = { 0 };

void EventHub_SendEvent(pHandle handle, pHandle senderHandle, uint32 msgValue)
{
    Identity *evtHubId = (Identity *)handle;
    if (!evtHubId) {
        return;
    }

    Request request = { 0 };
    request.msgId = EVENT_HUB_MSG_SEND_EVENT;
    request.data = senderHandle;
    request.len = 0;
    request.msgValue = msgValue;
    SAMGR_SendRequest(evtHubId, &request, NULL);
}

void EventHub_ListenToEvent(pHandle handle, Identity *senderId, Identity *receiverId,
                            uint32 msgId)
{
    Identity *evtHubId = (Identity *)handle;
    if (!evtHubId) {
        return;
    }

    EventHubReceiverNode *node =
            (EventHubReceiverNode *)MEM_Malloc(sizeof(EventHubReceiverNode));
    node->msgId = msgId;
    node->recvIdentity = *receiverId;
    node->sendIdentity = *senderId;

    LOGD("ListenToEvent sid %d, rid %d, squeueid=%p\n", node->sendIdentity.serviceId,
         node->recvIdentity.serviceId, node->sendIdentity.queueId);
    Request request = { 0 };
    request.msgId = EVENT_HUB_MSG_ADD_RECEIVER;
    request.data = node;
    request.len = sizeof(EventHubReceiverNode);
    request.msgValue = 0;
    SAMGR_SendRequest(evtHubId, &request, NULL);
}

void EventHub_AddSender(pHandle handle, Identity *senderId, pHandle senderHandle)
{
    // EventHubService *timerService = (EventHubService *)handle;
    Identity *evtHubId = (Identity *)handle;
    if (!evtHubId) {
        return;
    }

    EventHubSenderNode *node = (EventHubSenderNode *)MEM_Malloc(sizeof(EventHubSenderNode));
    node->handle = senderHandle;
    node->sendIdentity = *senderId;

    Request request = { 0 };
    request.msgId = EVENT_HUB_MSG_ADD_SENDER;
    request.data = node;
    request.len = sizeof(EventHubSenderNode);
    request.msgValue = 0;
    SAMGR_SendRequest(evtHubId, &request, NULL);
}

pHandle EventHub_GetInstance(void)
{
    if (g_evtHubId.queueId) {
        return &g_evtHubId;
    } else {
        SAMGR_GetInstance()->GetServiceId(EVENT_HUB_SERVICE, &g_evtHubId);
        return &g_evtHubId;
    }
}