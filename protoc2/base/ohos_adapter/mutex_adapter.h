#ifndef LITE_MUTEX_ADAPTER_H
#define LITE_MUTEX_ADAPTER_H

#include <ohos_types.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef void *MutexId;

MutexId MUTEX_Create(void);

void MUTEX_Delete(MutexId mutex);

void MUTEX_Lock(MutexId mutex);

void MUTEX_Unlock(MutexId mutex);

void MUTEX_InitGlobalLockValue(void);

void MUTEX_GlobalLock(void);

void MUTEX_GlobalUnlock(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif