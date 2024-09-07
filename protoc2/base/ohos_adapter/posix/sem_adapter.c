#include "semaphore.h"

#include "sem_adapter.h"
#include "stdio.h"
#include "stdlib.h"

SemId SEM_Create(uint32 value)
{
#ifdef __APPLE__
    char name[16];
    static int sn;
    sem_t *sem = NULL;
    snprintf(name, sizeof(name), "samgr_sem%d", sn++);
    sem = sem_open(name, O_CREAT, 0644, value);
    return (SemId)sem;
#else
    sem_t *sem = (sem_t *)malloc(sizeof(sem_t));
    // TODO: find free sem id
    sem_init(sem, 0, value);
    return (SemId)sem;
#endif
}

int SEM_Post(SemId id)
{
    return sem_post((sem_t *)id);
}

int SEM_Wait(SemId id)
{
    if (!id) {
        return -1;
    }
    return sem_wait((sem_t *)id);
}

int SEM_Destory(SemId id)
{
    int ret;

    ret = sem_close((sem_t *)id);
    free(id);

    return ret;
}