

#ifndef HIVIEWDFX_HILOG_TRACE_H
#define HIVIEWDFX_HILOG_TRACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int (*RegisterFunc)(uint64_t *, uint32_t *, uint64_t *, uint64_t *);

    int HiLogRegisterGetIdFun(RegisterFunc);
    void HiLogUnregisterGetIdFun(RegisterFunc);

#ifdef __cplusplus
}
#endif

#endif // HIVIEWDFX_HILOG_TRACE_H
