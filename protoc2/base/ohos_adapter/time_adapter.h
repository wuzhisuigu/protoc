
#ifndef LITE_TIME_ADAPTER_H
#define LITE_TIME_ADAPTER_H

#include <ohos_types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

    int32 WDT_Start(uint32 ms);

    int32 WDT_Reset(uint32 ms);

    int32 WDT_Stop(void);

    uint64 SAMGR_GetProcessTime(void);
    uint32 TIME_GetSysTime(void);
#define TIME_Diff(newTime, oldTime) (newTime > oldTime ? (newTime - oldTime) : (0xFFFFFFFF - oldTime + newTime))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_TIME_ADAPTER_H
