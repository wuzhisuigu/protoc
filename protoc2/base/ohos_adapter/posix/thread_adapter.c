
#include "thread_adapter.h"
#include "mutex_adapter.h"
#include <pthread.h>
#include <unistd.h>
#include "memory_adapter.h"
#define PRI_BUTT 39
#define MIN_STACK_SIZE 0x8000
static int g_threadCount = 0;
static pthread_key_t g_localKey = -1;
static pthread_once_t g_localKeyOnce = PTHREAD_ONCE_INIT;

static void KeyCreate()
{
    (void)pthread_key_create(&g_localKey, NULL);
}

ThreadId THREAD_Create(Runnable run, void *argv, const ThreadAttr *attr)
{
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setstacksize(&threadAttr, (attr->stackSize | MIN_STACK_SIZE));
#ifdef SAMGR_LINUX_ADAPTER
    struct sched_param sched = {attr->priority};
#else
    struct sched_param sched = {PRI_BUTT - attr->priority};
#endif
    pthread_attr_setinheritsched(&threadAttr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&threadAttr, SCHED_RR);
    pthread_attr_setschedparam(&threadAttr, &sched);
    (void)pthread_once(&g_localKeyOnce, KeyCreate);
    pthread_t threadId = 0;
    int err = pthread_create(&threadId, &threadAttr, run, argv);
    if (err != 0)
    {
        return NULL;
    }

    MUTEX_GlobalLock();
    g_threadCount++;
    MUTEX_GlobalUnlock();
    return (ThreadId)threadId;
}

int THREAD_Total(void)
{
    return g_threadCount;
}

void *THREAD_GetThreadLocal(void)
{
    return pthread_getspecific(g_localKey);
}

void THREAD_SetThreadLocal(const void *local)
{
    pthread_setspecific(g_localKey, local);
}

void THREAD_Sleep(uint32 ms)
{
    usleep(ms * 1000);
}

uint32 THREAD_GetTaskId(void)
{
    return (uint32)pthread_self();
}

uint32 THREAD_GetStackHighWaterMark()
{
    return 1024 * 1024;
}

int THREAD_GetThreadInfo(uint32 i, struct ThreadInfo *info)
{
    (void)i;
    (void)info;
    return -1;
}