
#include "message_inner.h"
#include <ohos_errno.h>
#include "memory_adapter.h"
#include "queue_adapter.h"
#include "string.h"
#include "task_manager.h"
#include "samgr_common.h"

#include "dfx_log.h"
#define DFX_LOG_MODULE_ID OHOS_MODULE_SAMGR

#define DONT_WAIT    0
#define WAIT_FOREVER (-1)
static int32 SharedSend(MQueueId queueId, Exchange *exchange, int initRef);
static BOOL FreeReference(Exchange *exchange);

int32 SAMGR_SendRequest(const Identity *identity, const Request *request, Handler handler)
{
    int ret = 0;
    if (request == NULL || identity == NULL) {
        return EC_INVALID;
    }

    Exchange exchange = { *identity, *request, { NULL, 0 }, MSG_NON, handler, NULL };
    exchange.id.queueId = NULL;
    if (handler != NULL) {
        exchange.id.queueId = SAMGR_GetCurrentQueueID();
        exchange.type = MSG_CON;
    }

    ret = QUEUE_Put(identity->queueId, &exchange, 0, DONT_WAIT);
    if (ret) {
        LOGE("QUEUE_Put to sid %d, return %d\n", identity->serviceId, ret);
    }
    return ret;
}

int32 SAMGR_SendResponse(const Request *request, const Response *response)
{
    // we need send the default the con message or not?
    if (request == NULL) {
        return EC_INVALID;
    }

    Exchange *exchange = GET_OBJECT(request, Exchange, request);
    if (exchange->type != MSG_CON) {
        return EC_INVALID;
    }

    if (exchange->handler == NULL) {
        return EC_SUCCESS;
    }

    exchange->type = MSG_ACK;
    exchange->response.data = NULL;
    exchange->response.len = 0;
    if (response != NULL) {
        exchange->response = *response;
    }

    // If there is no task queue, we will call the response processor in current task.
    if (exchange->id.queueId == NULL) {
        exchange->handler(&exchange->request, &exchange->response);
        return EC_SUCCESS;
    }

    // Send back to the origin to process the task.
    int32 ret = SharedSend(exchange->id.queueId, exchange, 1);
    if (ret != EC_SUCCESS) {
        exchange->handler(&exchange->request, &exchange->response);
        (void)FreeReference(exchange);
    }
    return EC_SUCCESS;
}

int32 SAMGR_MsgRecv(MQueueId queueId, uint8 *interMsg, uint32 size)
{
    if (queueId == NULL || interMsg == NULL || size == 0) {
        return EC_INVALID;
    }

    memset(interMsg, 0x00, size);

    return QUEUE_Pop(queueId, interMsg, 0, WAIT_FOREVER);
}

int32 SAMGR_FreeMsg(Exchange *exchange)
{
    if (!FreeReference(exchange)) {
        return EC_SUCCESS;
    }

    if (exchange->request.len > 0) {
        MEM_Free(exchange->request.data);
        exchange->request.data = NULL;
    }

    if (exchange->response.len > 0) {
        MEM_Free(exchange->response.data);
        exchange->response.data = NULL;
    }
    return EC_SUCCESS;
}

uint32 *SAMGR_SendSharedRequest(const Identity *identity, const Request *request, uint32 *token,
                                Handler handler)
{
    if (identity == NULL || request == NULL) {
        return NULL;
    }
    Exchange exchange = { *identity, *request, { NULL, 0 }, MSG_NON, handler, token };
    exchange.type = (handler == NULL) ? MSG_NON : MSG_CON;
    exchange.id.queueId = NULL;
    int32 err = SharedSend(identity->queueId, &exchange, 0);
    if (err != EC_SUCCESS) {
        HILOG_ERROR(HILOG_MODULE_SAMGR, "SharedSend [%p] failed(%d)!", identity->queueId, err);
        (void)FreeReference(&exchange);
    }
    return exchange.sharedRef;
}

int32 SAMGR_SendSharedDirectRequest(const Identity *id, const Request *req,
                                    const Response *resp, uint32 **ref, Handler handler)
{
    if (handler == NULL || ref == NULL) {
        return EC_INVALID;
    }

    Exchange exchange;
    memset(&exchange, 0, sizeof(exchange));
    if (req != NULL) {
        exchange.request = *req;
    }

    if (resp != NULL) {
        exchange.response = *resp;
    }

    exchange.handler = handler;
    exchange.sharedRef = *ref;
    exchange.type = MSG_DIRECT;
    exchange.id = *id;
    exchange.id.queueId = NULL;
    int32 err = SharedSend(id->queueId, &exchange, 0);
    if (err != EC_SUCCESS) {
        HILOG_ERROR(HILOG_MODULE_SAMGR, "SharedSend [%p] failed(%d)!", id->queueId, err);
        (void)FreeReference(&exchange);
    }
    *ref = exchange.sharedRef;
    return err;
}

int32 SAMGR_SendResponseByIdentity(const Identity *id, const Request *request,
                                   const Response *response)
{
    // we need send the default the con message or not?
    if (request == NULL || id == NULL) {
        return EC_INVALID;
    }

    Exchange *exchange = GET_OBJECT(request, Exchange, request);
    if (exchange->type == MSG_NON) {
        return EC_INVALID;
    }
    exchange->id.queueId = id->queueId;

    return SAMGR_SendResponse(request, response);
}

static int32 SharedSend(MQueueId queueId, Exchange *exchange, int initRef)
{
    /* if the msg data and response is NULL, we just direct copy, no need shared the message. */
    /* modify for samgr_rpc, if sharedRef is not null, inref it */
    if ((exchange->request.data == NULL || exchange->request.len <= 0) &&
        (exchange->response.data == NULL || exchange->response.len <= 0) &&
        (exchange->sharedRef == NULL)) {
        return QUEUE_Put(queueId, exchange, 0, DONT_WAIT);
    }

    /* 1.add reference */
    MUTEX_GlobalLock();
    if (exchange->sharedRef == NULL) {
        exchange->sharedRef = (uint32 *)MEM_Malloc(sizeof(uint32));
        if (exchange->sharedRef == NULL) {
            MUTEX_GlobalUnlock();
            return EC_NOMEMORY;
        }
        *(exchange->sharedRef) = initRef;
    }
    (*(exchange->sharedRef))++;
    MUTEX_GlobalUnlock();

    return QUEUE_Put(queueId, exchange, 0, DONT_WAIT);
}

static BOOL FreeReference(Exchange *exchange)
{
    if (exchange == NULL) {
        return FALSE;
    }

    BOOL needFree = TRUE;
    /* 1. check the shared reference */
    MUTEX_GlobalLock();
    if (exchange->sharedRef != NULL) {
        if (*(exchange->sharedRef) > 0) {
            (*(exchange->sharedRef))--;
        }

        if (*(exchange->sharedRef) > 0) {
            needFree = FALSE;
        }
    }
    MUTEX_GlobalUnlock();

    if (needFree) {
        MEM_Free(exchange->sharedRef);
        exchange->sharedRef = NULL;
    }
    return needFree;
}