
#include "time_adapter.h"
#include <ohos_errno.h>
#include <time.h>

#define MS_PER_SECOND 1000
#define NS_PER_MS 1000000

int32 WDT_Start(uint32 ms)
{
    return WDT_Reset(ms);
}

int32 WDT_Reset(uint32 ms)
{
    (void)ms;
    return EC_FAILURE;
}

int32 WDT_Stop(void)
{
    return EC_FAILURE;
}

uint32 TIME_GetSysTime(void)
{
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((ts.tv_sec * MS_PER_SECOND) + (ts.tv_nsec / NS_PER_MS));
}

uint64 SAMGR_GetProcessTime(void)
{
    struct timespec ts = {0, 0};
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((ts.tv_sec * MS_PER_SECOND) + (ts.tv_nsec / NS_PER_MS));
}
