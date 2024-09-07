
#ifndef LITE_MESSAGEINNER_H
#define LITE_MESSAGEINNER_H

#include "message.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

    enum ExchangeType
    {
        MSG_EXIT = -1,
        MSG_NON = 0,
        MSG_CON = 1,
        MSG_ACK = 2,
        MSG_SYNC = 3,
        MSG_DIRECT = 4,
    };

    enum ExchangeFlag
    {
        MSG_RPC_NONE = 0x0,
        MSG_RPC_SH = 0x1,
        MSG_RPC_DSP = 0x2,
    };

#define EXCHANGE_FLAG_MASK_RPC 0x3

// Will be used for message interaction, so use one-byte alignment
#pragma pack(1)
    typedef struct Exchange Exchange;
    struct Exchange
    {
        Identity id; /**< The target service or feature identity. */
        Request request;
        Response response;
        uint16 type;       /**< The exchange type. */
        Handler handler;   /**< async response or immediately request callback function */
        uint32 *sharedRef; /**< use to share the request and response for saving memory */
    };
#pragma pack()

    int32 SAMGR_MsgRecv(MQueueId queueId, uint8 *interMsg, uint32 size);

    /**
     * The function just release the Message->data and Message->sharedRef(use free), not release the msg entry.
     * If you alloc the msg on the heep, you should release it by yourself, you`d better alloc on the stack.
     * The function will be called automatically.
     * Do not call this function manually, except the SM_SendRequest return error!
     *
     **/
    int32 SAMGR_FreeMsg(Exchange *exchange);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_MESSAGEINNER_H
