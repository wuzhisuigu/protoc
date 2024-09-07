
#ifndef LITE_MEMORY_ADAPTER_H
#define LITE_MEMORY_ADAPTER_H

#include "ohos_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

void *MEM_Malloc(uint32 size);

void MEM_Free(void *buffer);

#define SAMGR_Malloc(x) MEM_Malloc(x)
#define SAMGR_Free(x)   MEM_Free(x)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_MEMORY_ADAPTER_H
