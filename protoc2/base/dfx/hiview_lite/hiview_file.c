#include "hiview_config.h"
#include "hiview_def.h"
#include "hiview_file.h"
#include "hiview_util.h"
#include "ohos_types.h"
// #include "securec.h"
// #include "memory_adapter.h"
#include "string.h"
// static uint16 GetReadCursor(HiviewFile *fp);

static uint32 GetDefineFileVersion(uint8 type)
{
    switch (type) {
    case HIVIEW_UE_EVENT_FILE:
        return HIVIEW_UE_EVENT_VER;
    case HIVIEW_STAT_EVENT_FILE:
        return HIVIEW_STATIC_EVENT_VER;
    case HIVIEW_FAULT_EVENT_FILE:
        return HIVIEW_FAULT_EVENT_VER;
    default:
        // non-event file
        return 0;
    }
}

boolean InitHiviewFile(HiviewFile *fp, HiviewFileType type, uint32 size)
{
    if (fp == NULL || fp->path == NULL) {
        return FALSE;
    }

    fp->fhandle = HIVIEW_FileOpen(fp->path);
    if (fp->fhandle < 0) {
        return FALSE;
    }

    HiviewFileHeader *pHeader = &(fp->header);
    FileHeaderCommon *pCommon = &(pHeader->common);
    pCommon->type = (uint8)type;
    pHeader->size = size + sizeof(HiviewFileHeader);
    // Create file for the first time
    if (ReadFileHeader(fp) == FALSE) {
        switch (pCommon->type) {
        case HIVIEW_LOG_TEXT_FILE:
            pCommon->prefix = HIVIEW_FILE_HEADER_PREFIX_TEXT;
            break;
        case HIVIEW_LOG_BIN_FILE:
            pCommon->prefix = HIVIEW_FILE_HEADER_PREFIX_LOG;
            break;
        case HIVIEW_FAULT_EVENT_FILE:
        case HIVIEW_UE_EVENT_FILE:
        case HIVIEW_STAT_EVENT_FILE:
            pCommon->prefix = HIVIEW_FILE_HEADER_PREFIX_EVENT;
            break;
        default:
            break;
        }
        pCommon->codeMainVersion = HIVIEW_FILE_HEADER_MAIN_VERSION;
        pCommon->codeSubVersion = HIVIEW_FILE_HEADER_SUB_VERSION;
        pCommon->defineFileVersion = GetDefineFileVersion(pCommon->type);
        pHeader->createTime = (uint32)(HIVIEW_GetCurrentTime() / MS_PER_SECOND);
        pHeader->rCursor = sizeof(HiviewFileHeader);
        pHeader->wCursor = sizeof(HiviewFileHeader);
        if (WriteFileHeader(fp) == FALSE) {
            return FALSE;
        }
    } else {
        // Version number may change after system upgrade
        pCommon->codeMainVersion = HIVIEW_FILE_HEADER_MAIN_VERSION;
        pCommon->codeSubVersion = HIVIEW_FILE_HEADER_SUB_VERSION;
        pCommon->defineFileVersion = GetDefineFileVersion(pCommon->type);
    }

    return TRUE;
}

boolean WriteFileHeader(HiviewFile *fp)
{
    if (fp == NULL || fp->fhandle < 0) {
        return FALSE;
    }

    int32 ret;
    if (HIVIEW_FileSeek(fp->fhandle, 0, HIVIEW_SEEK_SET) >= 0) {
        HiviewFileHeader tmpHeader = fp->header;
#if LITTLE_ENDIAN_SYSTEM
        tmpHeader.common.prefix = Change32Endian(tmpHeader.common.prefix);
        tmpHeader.common.defineFileVersion = Change32Endian(tmpHeader.common.defineFileVersion);
        tmpHeader.createTime = Change32Endian(tmpHeader.createTime);
#endif
        ret = HIVIEW_FileWrite(fp->fhandle, (uint8 *)&(tmpHeader), sizeof(HiviewFileHeader));
        if (ret == sizeof(HiviewFileHeader)) {
            return TRUE;
        }
    }

    return FALSE;
}

boolean ReadFileHeader(HiviewFile *fp)
{
    HiviewFileHeader h = { 0 };

    if (fp == NULL || fp->fhandle < 0) {
        return FALSE;
    }

    int32 ret;
    // uint32 t = (uint32)(HIVIEW_GetCurrentTime() / MS_PER_SECOND);
    if (HIVIEW_FileSeek(fp->fhandle, 0, HIVIEW_SEEK_SET) < 0) {
        return FALSE;
    }
    ret = HIVIEW_FileRead(fp->fhandle, (uint8 *)&h, sizeof(HiviewFileHeader));
#if LITTLE_ENDIAN_SYSTEM
    h.common.prefix = Change32Endian(h.common.prefix);
    h.common.defineFileVersion = Change32Endian(h.common.defineFileVersion);
    h.createTime = Change32Endian(h.createTime);
#endif
    h.wCursor = HIVIEW_FileSize(fp->fhandle);
    h.rCursor = sizeof(HiviewFileHeader);
    if ((ret == sizeof(HiviewFileHeader)) &&
        ((h.common.prefix & 0xFFFFFF00) == HIVIEW_FILE_HEADER_PREFIX_MASK) &&
        (h.wCursor >= sizeof(HiviewFileHeader))) {
        memcpy(&(fp->header), (void *)&h, sizeof(HiviewFileHeader));
        return TRUE;
    } else {
        return FALSE;
    }
}

int32 WriteToFile(HiviewFile *fp, const uint8 *data, uint32 len)
{
    if (fp == NULL || fp->fhandle < 0 || len == 0) {
        return 0;
    }
    int32 wLen = 0;
    HiviewFileHeader *h = &(fp->header);
    // overflow
    if ((h->wCursor + len > h->size) || (h->wCursor + len > g_hiviewConfig.logFileSizeMax)) {
        ProcFile(fp, fp->outPath, HIVIEW_FILE_RENAME);
        if (fp->pFunc != NULL) {
            fp->pFunc(fp->outPath, h->common.type, HIVIEW_FILE_FULL);
        }
    }
    if (HIVIEW_FileSeek(fp->fhandle, h->wCursor, HIVIEW_SEEK_SET) < 0) {
        return 0;
    }
    if ((int32)len == HIVIEW_FileWrite(fp->fhandle, data, len)) {
        h->wCursor += len;
        wLen += len;
    }
    return wLen;
}

int32 ReadFromFile(HiviewFile *fp, uint8 *data, uint32 readLen)
{
    if (fp == NULL || data == NULL || fp->fhandle < 0 || readLen == 0) {
        return 0;
    }

    HiviewFileHeader *h = &(fp->header);
    uint32 wCursor = h->wCursor;
    uint32 rCursor = h->rCursor;
    if (wCursor < readLen) {
        return 0;
    }
    int32 rLen = (readLen <= (wCursor - rCursor)) ? readLen : (wCursor - rCursor);
    if (HIVIEW_FileSeek(fp->fhandle, rCursor, HIVIEW_SEEK_SET) < 0) {
        return 0;
    }
    if ((int32)rLen == HIVIEW_FileRead(fp->fhandle, data, rLen)) {
        h->rCursor += rLen;
    } else {
        rLen = 0;
    }
    return rLen;
}

uint32 GetFileUsedSize(HiviewFile *fp)
{
    if (fp == NULL || fp->fhandle < 0) {
        return 0;
    }
    return fp->header.wCursor;
}

uint32 GetFileFreeSize(HiviewFile *fp)
{
    if (fp == NULL || fp->fhandle < 0) {
        return 0;
    }

    return (fp->header.size - fp->header.wCursor);
}

int32 CloseHiviewFile(HiviewFile *fp)
{
    if (fp != NULL && fp->fhandle > 0) {
        if (strcmp(fp->outPath, HIVIEW_FILE_OUT_PATH_LOG) != 0) {
            HIVIEW_MemFree(MEM_POOL_HIVIEW_ID, fp->outPath);
            fp->outPath = HIVIEW_FILE_OUT_PATH_LOG;
        } else if (strcmp(fp->outPath, HIVIEW_FILE_OUT_PATH_UE_EVENT) != 0) {
            HIVIEW_MemFree(MEM_POOL_HIVIEW_ID, fp->outPath);
            fp->outPath = HIVIEW_FILE_OUT_PATH_UE_EVENT;
        } else if (strcmp(fp->outPath, HIVIEW_FILE_OUT_PATH_FAULT_EVENT) != 0) {
            HIVIEW_MemFree(MEM_POOL_HIVIEW_ID, fp->outPath);
            fp->outPath = HIVIEW_FILE_OUT_PATH_FAULT_EVENT;
        } else if (strcmp(fp->outPath, HIVIEW_FILE_OUT_PATH_STAT_EVENT) != 0) {
            HIVIEW_MemFree(MEM_POOL_HIVIEW_ID, fp->outPath);
            fp->outPath = HIVIEW_FILE_OUT_PATH_STAT_EVENT;
        }
        return HIVIEW_FileClose(fp->fhandle);
    }
    return -1;
}

int8 ProcFile(HiviewFile *fp, const char *dest, FileProcMode mode)
{
    if (fp == NULL || fp->fhandle < 0 || HIVIEW_FileClose(fp->fhandle) != 0) {
        return -1;
    }

    // HIVIEW_MutexLockOrWait(fp->mutex, OUT_PATH_WAIT_TIMEOUT);
    HIVIEW_MutexLock(fp->mutex);
    switch (mode) {
    case HIVIEW_FILE_COPY: {
        int32 ret = HIVIEW_FileCopy(fp->path, dest);
        fp->fhandle = HIVIEW_FileOpen(fp->path);
        if (ret != 0 || fp->fhandle < 0) {
            HIVIEW_MutexUnlock(fp->mutex);
            HIVIEW_UartPrint("Procfile failed, type : HIVIEW_FILE_COPY");
            return -1;
        }
        break;
    }
    case HIVIEW_FILE_RENAME: {
        uint8 type = fp->header.common.type;
        uint32 size = fp->header.size - sizeof(HiviewFileHeader);
        int32 ret = HIVIEW_FileMove(fp->path, dest);
        if (InitHiviewFile(fp, type, size) == FALSE || ret != 0) {
            HIVIEW_MutexUnlock(fp->mutex);
            HIVIEW_UartPrint("Procfile failed, type : HIVIEW_FILE_RENAME");
            return -1;
        }
        break;
    }
    default:
        HIVIEW_MutexUnlock(fp->mutex);
        HIVIEW_UartPrint("Procfile failed, type : Unknown type");
        return -1;
    }
    HIVIEW_MutexUnlock(fp->mutex);
    return 0;
}

int IsValidPath(const char *path)
{
    if (strcmp(path, HIVIEW_FILE_PATH_LOG) == 0 ||
        strcmp(path, HIVIEW_FILE_PATH_UE_EVENT) == 0 ||
        strcmp(path, HIVIEW_FILE_PATH_FAULT_EVENT) == 0 ||
        strcmp(path, HIVIEW_FILE_PATH_STAT_EVENT) == 0 ||
        strcmp(path, HIVIEW_FILE_OUT_PATH_LOG) == 0 ||
        strcmp(path, HIVIEW_FILE_OUT_PATH_UE_EVENT) == 0 ||
        strcmp(path, HIVIEW_FILE_OUT_PATH_FAULT_EVENT) == 0 ||
        strcmp(path, HIVIEW_FILE_OUT_PATH_STAT_EVENT) == 0) {
        return -1;
    }
    return 0;
}

void RegisterFileWatcher(HiviewFile *fp, FileProc func, const char *path)
{
    if (fp == NULL || func == NULL || path == NULL) {
        return;
    }
    fp->pFunc = func;
    if (path == NULL || IsValidPath(path) != 0) {
        return;
    }

    int len = strlen(path) + 1;
    char *tmp = (char *)HIVIEW_MemAlloc(MEM_POOL_HIVIEW_ID, len);
    if (tmp == NULL) {
        return;
    }
    strncpy(tmp, path, len);
    fp->outPath = tmp;
}

void UnRegisterFileWatcher(HiviewFile *fp, FileProc func)
{
    (void)func;
    fp->pFunc = NULL;
    if (IsValidPath(fp->outPath) == 0) {
        HIVIEW_MemFree(MEM_POOL_HIVIEW_ID, fp->outPath);
    }
    switch (fp->header.common.type) {
    case HIVIEW_FAULT_EVENT_FILE:
        fp->outPath = HIVIEW_FILE_OUT_PATH_FAULT_EVENT;
        break;
    case HIVIEW_UE_EVENT_FILE:
        fp->outPath = HIVIEW_FILE_OUT_PATH_UE_EVENT;
        break;
    case HIVIEW_STAT_EVENT_FILE:
        fp->outPath = HIVIEW_FILE_OUT_PATH_STAT_EVENT;
        break;
    case HIVIEW_LOG_TEXT_FILE:
    case HIVIEW_LOG_BIN_FILE:
        fp->outPath = HIVIEW_FILE_OUT_PATH_LOG;
        break;
    default:
        break;
    }
}