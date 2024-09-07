

#ifndef LITE_TASKMANAGER_H
#define LITE_TASKMANAGER_H

#include <ohos_types.h>
#include "thread_adapter.h"
#include "mutex_adapter.h"
#include "service.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

#ifndef MAX_TASK_SIZE
#define MAX_TASK_SIZE 0xFFFFFF
#endif
#define MAX_REF_NUM 15
#define MSG_PROC_THRESHOLD (10 * 1000)
#define GET_REMAIN_TIME(time) ((0xFFFFFFFF - (time)) + 1)
#define GET_INTERVAL(last, now) (((last) > (now)) ? (GET_REMAIN_TIME(last) + (now)) : ((now) - (last)))

    typedef struct TaskPool TaskPool;
    struct TaskPool
    {
        MQueueId queueId;
        uint16 stackSize;
        uint8 priority; // task run priority
        uint8 size;
        uint8 top;
        int8 ref;
        ThreadId tasks[0];
    };
    TaskPool *SAMGR_CreateFixedTaskPool(const TaskConfig *config, const char *name, uint8 size);
    int32 SAMGR_StartTaskPool(TaskPool *pool, const char *name);
    int32 SAMGR_ReleaseTaskPool(TaskPool *pool);
    TaskPool *SAMGR_ReferenceTaskPool(TaskPool *pool);
    MQueueId SAMGR_GetCurrentQueueID(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // LITE_TASKMANAGER_H
