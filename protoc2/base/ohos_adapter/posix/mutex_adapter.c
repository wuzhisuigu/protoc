#include "mutex_adapter.h"
#include <pthread.h>
#include <unistd.h>
#include "memory_adapter.h"
#define PRI_BUTT 39
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

#define RECURSIVE

MutexId MUTEX_Create()
{
    // #if (sizeof(pthread_mutex_t) == sizeof(void*)）
#if 0
    pthread_mutex_t mutex;
    (void)pthread_mutex_init(&mutex, NULL);
#else
    pthread_mutex_t *mutex = MEM_Malloc(sizeof(pthread_mutex_t));
    if (mutex == NULL) {
        return NULL;
    }

#ifdef RECURSIVE

    pthread_mutexattr_t attr;
    int ret = 0;
    if ((ret = pthread_mutexattr_init(&attr)) != 0) {
        exit(1);
    }
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &attr);
#else
    (void)pthread_mutex_init(mutex, NULL);
#endif

#endif
    return (MutexId)mutex;
}

void MUTEX_Delete(MutexId mutex)
{
    pthread_mutex_destroy((pthread_mutex_t *)mutex);
    MEM_Free(mutex);
}

void MUTEX_Lock(MutexId mutex)
{
    if (mutex == NULL) {
        return;
    }
    pthread_mutex_lock((pthread_mutex_t *)mutex);
}

void MUTEX_Unlock(MutexId mutex)
{
    if (mutex == NULL) {
        return;
    }
    pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

void MUTEX_GlobalLock(void)
{
    pthread_mutex_lock(&g_mutex);
}

void MUTEX_GlobalUnlock(void)
{
    pthread_mutex_unlock(&g_mutex);
}

void MUTEX_InitGlobalLockValue(void)
{
    ;
}
