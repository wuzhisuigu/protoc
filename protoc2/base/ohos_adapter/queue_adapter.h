
#ifndef LITE_QUEUE_ADAPTER_H
#define LITE_QUEUE_ADAPTER_H

#include <ohos_types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif
    typedef void *MQueueId;
    MQueueId QUEUE_Create(const char *name, int size, int count);

    int QUEUE_Put(MQueueId queueId, const void *element, uint8 pri, int timeout);

    int QUEUE_Pop(MQueueId queueId, void *element, uint8 *pri, int timeout);

    int QUEUE_Destroy(MQueueId queueId);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_QUEUE_ADAPTER_H
