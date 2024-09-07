
#ifndef __SEM_ADAPTER_H__
#define __SEM_ADAPTER_H__

#include "ohos_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef void *SemId;


SemId SEM_Create(uint32 value);


int SEM_Post(SemId id);

int SEM_Wait(SemId id);

int SEM_Destory(SemId id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif
