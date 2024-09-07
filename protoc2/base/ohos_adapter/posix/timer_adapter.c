#include "timer_adapter.h"
#include <ohos_errno.h>

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define KAL_TASK_NAME_LEN 32
#define LOSCFG_BASE_CORE_MS_PER_SECOND 1000
#define OS_SYS_NS_PER_SECOND 1000000000

typedef enum
{
    KAL_TIMER_ONCE = 0,    // /< One-shot timer.
    KAL_TIMER_PERIODIC = 1 // /< Repeating timer.
} KAL_TimerType;

typedef struct
{
    timer_t timerPtr;
    KAL_TimerType type;
    unsigned int millisec;
    TimerFunc func;
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

TimerId TIMER_Create(TimerFunc func, uint32 interval, TimerAttr *attr)
{
    //   TimerHandle_t timer = xTimerCreate(attr->name, interval/portTICK_PERIOD_MS, attr->autoReload, attr->userData, func);
    //    return (TimerId)timer;
    //}
    // KalTimerId KalTimerCreate(KalTimerProc func, TimerFunc type, void* arg, unsigned int millisec)
    //{
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
    kalTimer->arg.sival_ptr = attr->userData;
    kalTimer->type = attr->autoReload ? KAL_TIMER_PERIODIC : KAL_TIMER_ONCE;
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
#else

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct Timer
{
    TimerFunc func;
    uint32 interval;
    TimerAttr attr;
    uint32 runninng;
    uint32 loop;
    pthread_t threadId;
} Timer;

void *timer_thread(void *arg)
{
    Timer *timer = (Timer *)arg;
    do
    {
        usleep(timer->interval * 1000);
        if (timer->runninng && timer->func)
        {
            timer->func(timer);
        }
    } while (timer->loop);
    return NULL;
}

TimerId TIMER_Create(TimerFunc func, uint32 interval, TimerAttr *attr)
{
    if (func == NULL || interval == 0)
    {
        return 0;
    }
    Timer *timer = (Timer *)malloc(sizeof(Timer));
    if (attr)
    {
        timer->attr = *attr;
    }
    else
    {
        timer->attr.autoReload = 0;
        timer->attr.name = "";
        timer->attr.userData = NULL;
    }
    timer->func = func;
    timer->interval = interval;
    timer->runninng = 0;
    timer->loop = 1;

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    int err = pthread_create(&timer->threadId, &threadAttr, timer_thread, timer);
    if (err != 0)
    {
        return NULL;
    }
    return timer;
}

void *TIMER_GetUserData(TimerId id)
{
    Timer *timer = (Timer *)id;
    return timer->attr.userData;
}

int TIMER_Start(TimerId id)
{
    Timer *timer = (Timer *)id;
    timer->runninng = 1;
    return 0;
}
int TIMER_Stop(TimerId id)
{
    Timer *timer = (Timer *)id;
    timer->runninng = 0;
    return 0;
}

int TIMER_Delete(TimerId id)
{
    Timer *timer = (Timer *)id;
    timer->runninng = 0;
    timer->loop = 0;
    pthread_join(timer->threadId, NULL);

    free(timer);
    return 0;
}

#endif