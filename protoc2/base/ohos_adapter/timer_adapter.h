#ifndef __TIMER_ADAPTER_H__
#define __TIMER_ADAPTER_H__

#include "ohos_types.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
typedef void *TimerId;

typedef void (*TimerFunc)(TimerId id);

typedef struct TimerAttr TimerAttr;
struct TimerAttr {
    const char *name;   // name of the thread
    BOOL autoReload;   // size of stack
    void* userData;
};

TimerId TIMER_Create(TimerFunc func,  uint32 interval, TimerAttr *attr);

void *TIMER_GetUserData(TimerId id);

int TIMER_Start(TimerId id);

int TIMER_Stop(TimerId id);

int TIMER_Delete(TimerId id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif