
#ifndef LITE_THREAD_ADAPTER_H
#define LITE_THREAD_ADAPTER_H

#include <ohos_types.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef void *ThreadId;

#define THREAD_NUM_MAX 16

/// \details Mutex ID identifies the mutex.
typedef void *(*Runnable)(void *argv);

typedef struct ThreadAttr ThreadAttr;
struct ThreadAttr {
    const char *name; // name of the thread
    uint32 stackSize; // size of stack
    uint8 priority; // initial thread priority
    uint8 reserved1; // reserved1 (must be 0)
    uint16 reserved2; // reserved2 (must be 0)
};
struct ThreadInfo {
    const char *name;
    uint32 stackSize; // in KB
    uint32 stackWM;
    uint16 state;
};

ThreadId THREAD_Create(Runnable run, void *argv, const ThreadAttr *attr);

int THREAD_Total(void);

uint32 THREAD_GetTaskId(void);

void THREAD_Sleep(uint32 ms);

void *THREAD_GetThreadLocal(void);

void THREAD_SetThreadLocal(const void *local);

int THREAD_GetThreadInfo(uint32 i, struct ThreadInfo *info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_THREAD_ADAPTER_H
