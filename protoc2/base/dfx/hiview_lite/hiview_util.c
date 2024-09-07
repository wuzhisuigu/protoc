#include "hiview_util.h"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "thread_adapter.h"
#include "mutex_adapter.h"
#include "memory_adapter.h"
#include "time_adapter.h"
#include "bsp_uart.h"

#define HIVIEW_WAIT_FOREVER      0xFFFFFFFF
#define HIVIEW_MS_PER_SECOND     1000
#define HIVIEW_NS_PER_MILISECOND 1000000
#define BUFFER_SIZE              128

void *HIVIEW_MemAlloc(uint8 modId, uint32 size)
{
    (void)modId;
    return MEM_Malloc(size);
}

void HIVIEW_MemFree(uint8 modId, void *pMem)
{
    (void)modId;
    MEM_Free(pMem);
}

int32 HIVIEW_RtcGetCurrentTime(uint64 *val, HIVIEW_RtcTime *time)
{
    (void)val;
    (void)time;
    return OHOS_SUCCESS;
}

HiviewMutexId_t HIVIEW_MutexInit()
{
    return (HiviewMutexId_t)MUTEX_Create();
}

void HIVIEW_MutexLock(HiviewMutexId_t mutex)
{
    if (mutex == NULL) {
        return;
    }
    MUTEX_Lock((MutexId)mutex);
}

void HIVIEW_MutexUnlock(HiviewMutexId_t mutex)
{
    if (mutex == NULL) {
        return;
    }
    MUTEX_Unlock((MutexId)mutex);
}

uint32 HIVIEW_GetTaskId()
{
    return (uint32)(unsigned long)THREAD_GetTaskId();
}

void HIVIEW_Sleep(uint32 ms)
{
    THREAD_Sleep(ms);
}

uint64 HIVIEW_GetCurrentTime()
{
    // can not use 32bit TIME_GetSysTime()
    // 32 bit timestamp can not represent upto years
    // 32 bit timestamp can represent upto 47 days

    // HIVIEW_GetCurrentTime will be used as the timestamp of logs.
    // need to use 64bit version and print full date
    return SAMGR_GetProcessTime();
}

void HIVIEW_UartPrint(const char *str)
{
    int len = strlen(str);
    bsp_debug_output(str, len);
}

#define USE_FFILE 1

int32 HIVIEW_FileOpen(const char *path)
{
    /* for MingW, we must set O_BINARY, otherwise the read will end unexpectly */
    /* and must set mode correctly, S_IRWXU | S_IRWXG, otherwise will have permission denied error*/
    return open(path, O_RDWR | O_CREAT | O_BINARY, S_IRWXU | S_IRWXG);
}

int32 HIVIEW_FileClose(int32 handle)
{
    return close(handle);
}

int32 HIVIEW_FileRead(int32 handle, uint8 *buf, uint32 len)
{
    return read(handle, (char *)buf, len);
}

int32 HIVIEW_FileWrite(int32 handle, const uint8 *buf, uint32 len)
{
    return write(handle, (const char *)buf, len);
}

int32 HIVIEW_FileSeek(int32 handle, int32 offset, int32 whence)
{
    return lseek(handle, (off_t)offset, whence);
}

int32 HIVIEW_FileSize(int32 handle)
{
    return lseek(handle, 0, SEEK_END);
}

int32 HIVIEW_FileUnlink(const char *path)
{
    return unlink(path);
}

int32 HIVIEW_FileCopy(const char *src, const char *dest)
{
    if (src == NULL || dest == NULL) {
        HIVIEW_UartPrint("HIVIEW_FileCopy input param is NULL");
        return -1;
    }
    int32 fdSrc = open(src, O_RDONLY | O_BINARY, 0);
    if (fdSrc < 0) {
        HIVIEW_UartPrint("HIVIEW_FileCopy open src file fail");
        return fdSrc;
    }

    /* for MingW, we must set O_BINARY, otherwise the read will end unexpectly */
    /* and must set mode correctly, S_IRWXU | S_IRWXG, otherwise will have permission denied error*/
    int32 fdDest = open(dest, O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IRWXU | S_IRWXG);
    if (fdDest < 0) {
        HIVIEW_UartPrint("HIVIEW_FileCopy open dest file fail");
        HIVIEW_FileClose(fdSrc);
        return fdDest;
    }
    boolean copyFailed = TRUE;
    uint8 *dataBuf = (uint8 *)HIVIEW_MemAlloc(MEM_POOL_HIVIEW_ID, BUFFER_SIZE);
    if (dataBuf == NULL) {
        HIVIEW_UartPrint("HIVIEW_FileCopy malloc erro");
        goto MALLOC_ERROR;
    }
    int32 nLen = HIVIEW_FileRead(fdSrc, dataBuf, BUFFER_SIZE);
    while (nLen > 0) {
        if (HIVIEW_FileWrite(fdDest, dataBuf, nLen) != nLen) {
            goto EXIT;
        }
        nLen = HIVIEW_FileRead(fdSrc, dataBuf, BUFFER_SIZE);
    }
    copyFailed = (nLen < 0);

EXIT:
    free(dataBuf);
MALLOC_ERROR:
    HIVIEW_FileClose(fdSrc);
    HIVIEW_FileClose(fdDest);
    if (copyFailed) {
        HIVIEW_UartPrint("HIVIEW_FileCopy copy failed");
        HIVIEW_FileUnlink(dest);
        return -1;
    }

    return 0;
}

int32 HIVIEW_FileMove(const char *src, const char *dest)
{
    int32 ret = HIVIEW_FileCopy(src, dest);
    if (HIVIEW_FileUnlink(src) != 0 || ret != 0) {
        return -1;
    }
    return 0;
}

void HIVIEW_WatchDogSystemReset()
{
    /* reset MCU Core */
}

uint8 HIVIEW_WdgResetFlag()
{
    /* Depend:HAL_WdgGetResetFlag */
    return 1;
}

uint32 Change32Endian(uint32 num)
{
    unsigned char *buffer = (unsigned char *)&num;
    uint32 newEndian = (buffer[3] & 0xFF);
    newEndian |= ((buffer[2] << 8) & 0xFF00);
    newEndian |= ((buffer[1] << 16) & 0xFF0000);
    newEndian |= ((buffer[0] << 24) & 0xFF000000);
    return newEndian;
}

uint16 Change16Endian(uint16 num)
{
    unsigned char *buffer = (unsigned char *)&num;
    uint16 newEndian = (buffer[1] & 0xFF);
    newEndian |= ((buffer[0] << 8) & 0xFF00);
    return newEndian;
}