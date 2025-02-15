#include "kal.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define KAL_TASK_NAME_LEN 32
#define LOSCFG_BASE_CORE_MS_PER_SECOND 1000
#define OS_SYS_NS_PER_SECOND 1000000000

typedef struct
{
    timer_t timerPtr;
    KalTimerType type;
    unsigned int millisec;
    KalTimerProc func;
    union sigval arg;
    int isRunning;
} KalTimer;

static void KalFunction(union sigval kalTimer)
{
    KalTimer *tmpPtr = (KalTimer *)(kalTimer.sival_ptr);
    if (tmpPtr->type == KAL_TIMER_ONCE)
    {
        tmpPtr->isRunning = 0;
    }
    tmpPtr->func(tmpPtr->arg);
}

static void KalMs2TimeSpec(struct timespec *tp, unsigned int ms)
{
    tp->tv_sec = ms / LOSCFG_BASE_CORE_MS_PER_SECOND;
    ms -= tp->tv_sec * LOSCFG_BASE_CORE_MS_PER_SECOND;
    tp->tv_nsec = (long)(((unsigned long long)ms * OS_SYS_NS_PER_SECOND) / LOSCFG_BASE_CORE_MS_PER_SECOND);
}

KalTimerId KalTimerCreate(KalTimerProc func, KalTimerType type, void *arg, unsigned int millisec)
{
    struct sigevent evp = {0};
    timer_t timer;
    if ((func == NULL) || ((type != KAL_TIMER_ONCE) && (type != KAL_TIMER_PERIODIC)))
    {
        return NULL;
    }
    KalTimer *kalTimer = (KalTimer *)malloc(sizeof(KalTimer));
    if (kalTimer == NULL)
    {
        return NULL;
    }
    kalTimer->func = func;
    kalTimer->arg.sival_ptr = arg;
    kalTimer->type = type;
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = KalFunction;
    evp.sigev_value.sival_ptr = kalTimer;
    int ret = timer_create(CLOCK_REALTIME, &evp, &timer);
    if (ret != 0)
    {
        free(kalTimer);
        return NULL;
    }
    kalTimer->timerPtr = timer;
    kalTimer->millisec = millisec;
    kalTimer->isRunning = 0;
    return (KalTimerId)kalTimer;
}

KalErrCode KalTimerStart(KalTimerId timerId)
{
    if (timerId == NULL)
    {
        return KAL_ERR_PARA;
    }
    struct itimerspec ts = {0};
    KalTimer *tmpPtr = (KalTimer *)timerId;
    KalMs2TimeSpec(&ts.it_value, tmpPtr->millisec);
    if (tmpPtr->type == KAL_TIMER_PERIODIC)
    {
        KalMs2TimeSpec(&ts.it_interval, tmpPtr->millisec);
    }
    int ret = timer_settime(tmpPtr->timerPtr, 0, &ts, NULL);
    if (ret != 0)
    {
        return KAL_ERR_PARA;
    }
    if (tmpPtr->millisec != 0)
    {
        tmpPtr->isRunning = 1;
    }
    return KAL_OK;
}

KalErrCode KalTimerChange(KalTimerId timerId, unsigned int millisec)
{
    if (timerId == NULL)
    {
        return KAL_ERR_PARA;
    }
    KalTimer *tmpPtr = (KalTimer *)timerId;
    struct itimerspec ts = {0};
    tmpPtr->millisec = millisec;
    if (tmpPtr->isRunning == 1)
    {
        KalMs2TimeSpec(&ts.it_value, millisec);
        if (tmpPtr->type == KAL_TIMER_PERIODIC)
        {
            KalMs2TimeSpec(&ts.it_interval, millisec);
        }
        int ret = timer_settime(tmpPtr->timerPtr, 0, &ts, NULL);
        if (ret != 0)
        {
            tmpPtr->isRunning = 0;
            return KAL_ERR_INNER;
        }
        if (millisec == 0)
        {
            tmpPtr->isRunning = 0;
        }
    }
    return KAL_OK;
}

KalErrCode KalTimerStop(KalTimerId timerId)
{
    if (timerId == NULL)
    {
        return KAL_ERR_PARA;
    }
    KalTimer *tmpPtr = (KalTimer *)timerId;
    struct itimerspec ts = {0};
    int ret = timer_settime(tmpPtr->timerPtr, 0, &ts, NULL);
    if (ret != 0)
    {
        return KAL_ERR_INNER;
    }
    tmpPtr->isRunning = 0;
    return KAL_OK;
}

KalErrCode KalTimerDelete(KalTimerId timerId)
{
    if (timerId == NULL)
    {
        return KAL_ERR_PARA;
    }
    KalTimer *tmpPtr = (KalTimer *)timerId;
    int ret = timer_delete(tmpPtr->timerPtr);
    free(timerId);
    return (ret != 0) ? KAL_ERR_INNER : KAL_OK;
}

unsigned int KalTimerIsRunning(KalTimerId timerId)
{
    if (timerId == NULL)
    {
        return 0;
    }
    return ((KalTimer *)timerId)->isRunning;
}