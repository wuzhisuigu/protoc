
#include "lock_free_queue.h"
#include <ohos_errno.h>
#include <memory_adapter.h>
#include <string.h>

LockFreeQueue *LFQUE_Create(int size, int count)
{
    if (size <= 0 || count <= 0)
    {
        return NULL;
    }

    int total = size * count + 1;
    if (total <= 0)
    {
        return NULL;
    }

    register LockFreeQueue *queue = (LockFreeQueue *)MEM_Malloc(sizeof(LockFreeQueue) + total);
    if (queue == NULL)
    {
        return NULL;
    }
    queue->write = 0;
    queue->read = 0;
    queue->itemSize = size;
    queue->totalSize = total;
    return queue;
}

BOOL LFQUE_IsFull(LockFreeQueue *queue)
{
    uint32 nextWrite = queue->write + 1;
    if (nextWrite >= queue->totalSize)
    {
        nextWrite = 0;
    }
    return (nextWrite == queue->read);
}

BOOL LFQUE_IsEmpty(LockFreeQueue *queue)
{
    return (queue->write == queue->read);
}

int LFQUE_Push(LockFreeQueue *queue, const void *element, uint8 pri)
{
    (void)pri;
    if (queue == NULL || element == NULL)
    {
        return EC_INVALID;
    }

    if (LFQUE_IsFull(queue))
    {
        return EC_BUSBUSY;
    }
    uint32 copyLen = (queue->totalSize - queue->write < queue->itemSize) ? (queue->totalSize - queue->write) : queue->itemSize;
    memcpy(&queue->buffer[queue->write], element, copyLen);

    element += copyLen;
    copyLen = queue->itemSize - copyLen;
    if (copyLen > 0)
    {
        memcpy(queue->buffer, element, copyLen);
    }

    uint32 write = queue->write + queue->itemSize;
    if (write >= queue->totalSize)
    {
        write = write - queue->totalSize;
    }
    queue->write = write;
    return EC_SUCCESS;
}

int LFQUE_Pop(LockFreeQueue *queue, void *element, uint8 *pri)
{
    (void)pri;
    if (queue == NULL || element == NULL)
    {
        return EC_INVALID;
    }
    if (LFQUE_IsEmpty(queue))
    {
        return EC_FAILURE;
    }

    uint32 copyLen = (queue->totalSize - queue->read < queue->itemSize) ? (queue->totalSize - queue->read) : queue->itemSize;
    memcpy(element, &queue->buffer[queue->read], copyLen);

    element += copyLen;
    copyLen = queue->itemSize - copyLen;
    if (copyLen > 0)
    {
        memcpy(element, queue->buffer, copyLen);
    }

    uint32 read = queue->read + queue->itemSize;
    if (read >= queue->totalSize)
    {
        read = read - queue->totalSize;
    }
    queue->read = read;
    return EC_SUCCESS;
}