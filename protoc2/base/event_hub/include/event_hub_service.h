#ifndef __EVENT_HUB_H__
#define __EVENT_HUB_H__

#include "message.h"
#include "ohos_types.h"
#include "utils_list.h"
#define EVENT_HUB_SERVICE "event_hub"

typedef enum EventHubMessage
{
    EVENT_HUB_MSG_ADD_SENDER,
    EVENT_HUB_MSG_ADD_RECEIVER,
    EVENT_HUB_MSG_SEND_EVENT,
    /** Maximum number of message IDs */
    EVENT_HUB_MSG_BUTT
} EventHubMessage;

typedef struct EventHubReceiverNode_s
{
    UTILS_DL_LIST link;
    Identity sendIdentity;
    Identity recvIdentity;
    uint32 msgId;
} EventHubReceiverNode;

typedef struct EventHubSenderNode_s
{
    UTILS_DL_LIST link;
    pHandle handle;
    Identity sendIdentity;
} EventHubSenderNode;

#ifdef __cplusplus
extern "C"
{
#endif

    pHandle EventHub_GetInstance(void);
    void EventHub_SendEvent(pHandle handle, pHandle senderHandle, uint32 msgValue);
    void EventHub_ListenToEvent(pHandle handle, Identity *senderId, Identity *receiverId, uint32 msgId);
    void EventHub_AddSender(pHandle handle, Identity *senderId, pHandle senderHandle);

#ifdef __cplusplus
}
#endif

#endif