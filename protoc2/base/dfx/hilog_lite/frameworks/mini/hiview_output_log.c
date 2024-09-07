#include "hiview_output_log.h"
#include "hiview_cache.h"
#include "hiview_config.h"
#include "hiview_def.h"
#include "hiview_file.h"
#include "hiview_log.h"
#include "hiview_log_limit.h"
#include "hiview_fifo_log.h"
#include "hiview_service.h"
#include "hiview_util.h"
#include "message.h"
#include "ohos_types.h"
#include "stdio.h"
#include "string.h"
#include <time.h>

#define SINGLE_FMT_MAX_LEN    8
#define FMT_CONVERT_TRMINATOR 2

#define ENABLE_OHOS_HIEVIEW_LOG_FILE 1
#define ENABLE_OHOS_HIEVIEW_LOG_TIME 1

static const char g_logLevelInfo[HILOG_LV_MAX] = {
    'N', // "NONE"
    'D', // "DEBUG"
    'I', // "INFO"
    'W', // "WARN"
    'E', // "ERROR"
    'F' // "FATAL"
};

#ifndef DISABLE_HILOG_CACHE
static uint8 g_logCacheBuffer[LOG_STATIC_CACHE_SIZE];
#endif
static HiviewCache g_logCache = {
    .size = 0,
    .buffer = NULL,
};

#if ENABLE_OHOS_HIEVIEW_LOG_FILE
static HiviewFile g_logFile = {
    .path = HIVIEW_FILE_PATH_LOG,
    .outPath = HIVIEW_FILE_OUT_PATH_LOG,
    .pFunc = NULL,
    .mutex = NULL,
    .fhandle = -1,
};
#endif

typedef struct LogFlushInfo LogFlushInfo;
struct LogFlushInfo {
    HiviewMutexId_t mutex;
};
static LogFlushInfo g_logFlushInfo;
static HilogProc g_hilogOutputProc = NULL;

typedef struct OutputLogInfo OutputLogInfo;
struct OutputLogInfo {
    HiviewMutexId_t mutex;
};
static OutputLogInfo g_outputLogInfo;

/* Output the log to UART using plaintext. */
static void OutputLogRealtime(const Request *req);

#if ENABLE_OHOS_HIEVIEW_LOG_FILE
/* Output the log to FLASH using text. */
static void OutputLog2TextFile(const Request *req);
/* Output the log to FLASH using binary. */
static void OutputLog2BinFile(const Request *req);
#endif
static int32 LogCommonFmt(char *outStr, int32 outStrlen, const HiLogCommon *commonContentPtr);
static int32 LogValuesFmt(char *desStrPtr, int32 desLen, const HiLogContent *logContentPtr);
static int32 LogDebugValuesFmt(char *desStrPtr, int32 desLen,
                               const HiLogContent *logContentPtr);
static int32 LogValuesFmtHash(char *desStrPtr, int32 desLen, const HiLogContent *logContentPtr);

void InitCoreLogOutput(void)
{
    g_logFlushInfo.mutex = HIVIEW_MutexInit();
    g_outputLogInfo.mutex = HIVIEW_MutexInit();
#ifndef DISABLE_HILOG_CACHE
    InitHiviewStaticCache(&g_logCache, LOG_CACHE, g_logCacheBuffer, sizeof(g_logCacheBuffer));
#endif
#if ENABLE_OHOS_HIEVIEW_LOG_FILE
    HiviewRegisterMsgHandle(HIVIEW_MSG_OUTPUT_LOG_TEXT_FILE, OutputLog2TextFile);
    HiviewRegisterMsgHandle(HIVIEW_MSG_OUTPUT_LOG_BIN_FILE, OutputLog2BinFile);
#endif
    HiviewRegisterMsgHandle(HIVIEW_MSG_OUTPUT_LOG_FLOW, OutputLogRealtime);
}

void InitLogOutput(void)
{
#if ENABLE_OHOS_HIEVIEW_LOG_FILE
    int8 opt = GETOPTION(g_hiviewConfig.outputOption);
    if (opt == OUTPUT_OPTION_DEBUG || opt == OUTPUT_OPTION_FLOW) {
        return;
    }
    HiviewFileType type = HIVIEW_LOG_TEXT_FILE;
    if (opt == OUTPUT_OPTION_BIN_FILE) {
        type = HIVIEW_LOG_BIN_FILE;
    }
    if (InitHiviewFile(&g_logFile, type,
                       (HIVIEW_LOG_FILE_SIZE / sizeof(HiLogContent)) * sizeof(HiLogContent)) ==
        FALSE) {
        HIVIEW_UartPrint("Open file[HIVIEW_LOG_BIN_FILE] failed.");
    }
    g_logFile.mutex = g_outputLogInfo.mutex;
#endif

#ifdef __HIVIEW_FIFO_LOG_H__
    fifo_log_init(HIVIEW_FIFO_LOG_TRT);
#endif
}

void ClearLogOutput(void)
{
#if ENABLE_OHOS_HIEVIEW_LOG_FILE
    int8 opt = GETOPTION(g_hiviewConfig.outputOption);
    if (g_logCache.usedSize > 0) {
        if (opt == OUTPUT_OPTION_TEXT_FILE) {
            OutputLog2TextFile(NULL);
        } else if (opt == OUTPUT_OPTION_BIN_FILE) {
            OutputLog2BinFile(NULL);
        }
    }
    CloseHiviewFile(&g_logFile);
#endif
}

void OutputLog(const uint8 *data, uint32 len)
{
    if (data == NULL) {
        return;
    }

    HiLogContent *hiLogContent = (HiLogContent *)data;
    if (g_hilogOutputProc != NULL) {
        if (g_hilogOutputProc(hiLogContent, len) == TRUE) {
            return;
        }
    }

#ifdef DISABLE_HILOG_CACHE
    boolean isDisableCache = TRUE;
#else
    boolean isDisableCache = FALSE;
#endif

#ifdef DISABLE_HILOG_LITE_PRINT_LIMIT
    boolean isDisablePrintLimited = TRUE;
#else
    boolean isDisablePrintLimited = FALSE;
#endif
    boolean isLogLimited = LogIsLimited(hiLogContent->commonContent.module);

    if (!isDisablePrintLimited && isLogLimited) {
        // The console output adopts the same restriction strategy as the file output,
        // and the log output to the file is restricted.
        return;
    }

    int8 opt = GETOPTION(g_hiviewConfig.outputOption);
    boolean isPrint = g_hiviewConfig.outputOption >= OUTPUT_OPTION_PRINT;
    if (opt == OUTPUT_OPTION_DEBUG || isPrint || isDisableCache) {
        char tempOutStr[LOG_FMT_MAX_LEN] = { 0 };
        if (LogContentFmt(tempOutStr, sizeof(tempOutStr), data) > 0) {
            HIVIEW_UartPrint(tempOutStr);
        }
    }

    if (opt == OUTPUT_OPTION_DEBUG || isDisableCache || isLogLimited) {
        return;
    }

    /* When the init of kernel is not finished, data is cached in the cache. */
    if (g_hiviewConfig.hiviewInited == FALSE) {
        if (WriteToCache(&g_logCache, data, len) != (int32)len) {
            HIVIEW_UartPrint("Write log to cache failed.");
        }
        return;
    }

    boolean writeFail = FALSE;
    if (WriteToCache(&g_logCache, (uint8 *)data, len) != (int32)len) {
        HIVIEW_UartPrint("Hilog writeToCache error!\n");
        writeFail = TRUE;
    }
    if (g_logCache.usedSize >= HIVIEW_HILOG_FILE_BUF_SIZE) {
        switch (opt) {
        case OUTPUT_OPTION_TEXT_FILE:
            HiviewSendMessage(HIVIEW_SERVICE, HIVIEW_MSG_OUTPUT_LOG_TEXT_FILE, 0);
            break;
        case OUTPUT_OPTION_BIN_FILE:
            HiviewSendMessage(HIVIEW_SERVICE, HIVIEW_MSG_OUTPUT_LOG_BIN_FILE, 0);
            break;
        case OUTPUT_OPTION_FLOW:
            HiviewSendMessage(HIVIEW_SERVICE, HIVIEW_MSG_OUTPUT_LOG_FLOW, 0);
            break;
        default:
            break;
        }
    }

    /* If the cache fails to be written, write the cache again. */
    if (writeFail) {
        WriteToCache(&g_logCache, (uint8 *)data, len);
    }
}

static void OutputLogRealtime(const Request *req)
{
    int8 opt = GETOPTION(g_hiviewConfig.outputOption);
    if (opt != OUTPUT_OPTION_FLOW) {
        // don't steal logs when output is not OUTPUT_OPTION_FLOW
        return;
    }

    HIVIEW_MutexLock(g_logFlushInfo.mutex);
    HiLogContent logContent;
    char tempOutStr[LOG_FMT_MAX_LEN] = { 0 };
    int32 len;
    (void)req;

    while (ReadFromCache(&g_logCache, (uint8 *)&(logContent.commonContent),
                         sizeof(HiLogCommon)) == sizeof(HiLogCommon)) {
        if (logContent.commonContent.head != LOG_INFO_HEAD) {
            DiscardCacheData(&g_logCache);
            HIVIEW_UartPrint("Discard cache[LOG_CACHE] data.");
            break;
        }
        len = logContent.commonContent.valueNumber * sizeof(uintptr);
        if (len > 0 && ReadFromCache(&g_logCache, (uint8 *)&(logContent.values), len) != len) {
            continue;
        }
        len = LogContentFmt(tempOutStr, sizeof(tempOutStr), (uint8 *)&logContent);
        if (len <= 0) {
            continue;
        }
        HIVIEW_UartPrint(tempOutStr);
    }
// read trt log
#ifdef __HIVIEW_FIFO_LOG_H__
    int ret = EC_SUCCESS;

    while (ret == EC_SUCCESS) {
        len = LOG_FMT_MAX_LEN;
        // the fifo_log producer need to make sure the log is null terminated
        ret = fifo_log_read(HIVIEW_FIFO_LOG_TRT, tempOutStr, len);
        if (ret == EC_SUCCESS) {
            HIVIEW_UartPrint(tempOutStr);
        }
    }

#endif

    HIVIEW_MutexUnlock(g_logFlushInfo.mutex);
}
#if ENABLE_OHOS_HIEVIEW_LOG_FILE
static void OutputLog2TextFile(const Request *req)
{
    int8 opt = GETOPTION(g_hiviewConfig.outputOption);
    if (opt != OUTPUT_OPTION_TEXT_FILE) {
        // don't steal logs when output is not OUTPUT_OPTION_FLOW
        return;
    }
    HIVIEW_MutexLock(g_logFlushInfo.mutex);
    HiLogContent logContent;
    char tempOutStr[LOG_FMT_MAX_LEN] = { 0 };
    (void)req;

    if (g_logCache.usedSize < sizeof(HiLogCommon)) {
        HIVIEW_MutexUnlock(g_logFlushInfo.mutex);
        return;
    }

    int32 len;
    while (ReadFromCache(&g_logCache, (uint8 *)&(logContent.commonContent),
                         sizeof(HiLogCommon)) == sizeof(HiLogCommon)) {
        if (logContent.commonContent.head != LOG_INFO_HEAD) {
            DiscardCacheData(&g_logCache);
            HIVIEW_UartPrint("Discard cache[LOG_CACHE] data.");
            break;
        }
        len = logContent.commonContent.valueNumber * sizeof(uintptr);
        if (len > 0 && ReadFromCache(&g_logCache, (uint8 *)&(logContent.values), len) != len) {
            continue;
        }
        len = LogContentFmt(tempOutStr, sizeof(tempOutStr), (uint8 *)&logContent);
        if (len > 0 && tempOutStr[len - 1] == '\0') {
            // prevent writing '\0' character to file
            len--;
        }
        if (len > 0 && WriteToFile(&g_logFile, (uint8 *)tempOutStr, len) != len) {
            g_hiviewConfig.writeFailureCount++;
        }
    }

// read trt log
#ifdef __HIVIEW_FIFO_LOG_H__
    int ret = EC_SUCCESS;

    while (ret == EC_SUCCESS) {
        len = LOG_FMT_MAX_LEN;
        // the fifo_log producer need to make sure the log is null terminated
        ret = fifo_log_read(HIVIEW_FIFO_LOG_TRT, tempOutStr, len);
        if (ret == EC_SUCCESS) {
            len = strnlen(tempOutStr, len);
            WriteToFile(&g_logFile, (uint8 *)tempOutStr, len);
        }
    }

#endif
    HIVIEW_MutexUnlock(g_logFlushInfo.mutex);
}

static void OutputLog2BinFile(const Request *req)
{
    HIVIEW_MutexLock(g_logFlushInfo.mutex);
    HiLogCommon *pCommonContent = NULL;
    uint16 len = 0;
    uint16 valueLen;
    uint8 *tmpBuffer = NULL;
    uint16 outputSize = g_logCache.usedSize;
    (void)req;

    if (outputSize < sizeof(HiLogCommon)) {
        HIVIEW_MutexUnlock(g_logFlushInfo.mutex);
        return;
    }
    tmpBuffer = (uint8 *)HIVIEW_MemAlloc(MEM_POOL_HIVIEW_ID, outputSize);
    if (tmpBuffer == NULL) {
        HIVIEW_MutexUnlock(g_logFlushInfo.mutex);
        return;
    }
    while (g_logCache.usedSize >= sizeof(HiLogCommon) &&
           outputSize > (len + sizeof(HiLogCommon))) {
        if (ReadFromCache(&g_logCache, tmpBuffer + len, sizeof(HiLogCommon)) !=
            sizeof(HiLogCommon)) {
            continue;
        }
        pCommonContent = (HiLogCommon *)(tmpBuffer + len);
        len += sizeof(HiLogCommon);
        if (pCommonContent->head != LOG_INFO_HEAD) {
            DiscardCacheData(&g_logCache);
            HIVIEW_UartPrint("Discard cache[LOG_CACHE] data.");
            break;
        }
        valueLen = pCommonContent->valueNumber * sizeof(uintptr);
        if (valueLen > 0) {
            if (ReadFromCache(&g_logCache, tmpBuffer + len, valueLen) != valueLen) {
                continue;
            }
            len += valueLen;
        }
    }
    if (len > 0 && WriteToFile(&g_logFile, tmpBuffer, len) != len) {
        g_hiviewConfig.writeFailureCount++;
        HIVIEW_UartPrint("Failed to write log data.");
    }
    HIVIEW_MemFree(MEM_POOL_HIVIEW_ID, tmpBuffer);
    HIVIEW_MutexUnlock(g_logFlushInfo.mutex);
}

uint32 GetLogFileSize(void)
{
    return GetFileUsedSize(&g_logFile);
}

uint32 ReadLogFile(uint8 *buf, uint32 len)
{
    if (buf == NULL) {
        return 0;
    }
    uint32 usedSize = GetFileUsedSize(&g_logFile);
    if (usedSize < len) {
        len = usedSize;
    }
    if (ReadFromFile(&g_logFile, buf, len) != (int32)len) {
        return 0;
    }

    return len;
}
#endif

int32 LogContentFmt(char *outStr, int32 outStrLen, const uint8 *pLogContent)
{
    int32 len;
    HiLogContent *logContentPtr = (HiLogContent *)pLogContent;

    len = LogCommonFmt(outStr, outStrLen, &(logContentPtr->commonContent));
    boolean isHash = CHECK_HASH_FLAG(logContentPtr->commonContent.level);
    if (len >= 0) {
        if (isHash) {
            len += LogValuesFmtHash(outStr + len, outStrLen - len - 2, logContentPtr);
        } else if (GETOPTION(g_hiviewConfig.outputOption) == OUTPUT_OPTION_DEBUG) {
            len += LogDebugValuesFmt(outStr + len, outStrLen - len - 2, logContentPtr);
        } else {
            len += LogValuesFmt(outStr + len, outStrLen - len - 2, logContentPtr);
        }
    }

    if (len < 0) {
        return len;
    }

    if (len >= outStrLen - TAIL_LINE_BREAK) {
        outStr[outStrLen - 3] = '\r';
        outStr[outStrLen - 2] = '\n';
        outStr[outStrLen - 1] = '\0';
    } else {
        if (outStr[len - 1] == '\n') {
            outStr[len - 1] = '\r';
            outStr[len++] = '\n';
            outStr[len++] = '\0';
        } else {
            outStr[len++] = '\r';
            outStr[len++] = '\n';
            outStr[len++] = '\0';
        }
    }

    return len;
}

static int32 LogCommonFmt(char *outStr, int32 outStrLen, const HiLogCommon *commonContentPtr)
{
    int32 ret = 0;
#if ENABLE_OHOS_HIEVIEW_LOG_TIME
    time_t time;
    uint32 year, month, day, hour, min, sec;
    uint8_t level;
    struct tm nowTime = { 0 };

    time = commonContentPtr->time;
#if __WIN64
    localtime_s(&nowTime, &time);
#else
    localtime_r(&time, &nowTime);
#endif
    year = nowTime.tm_year + 1900;
    month = nowTime.tm_mon + 1;
    day = nowTime.tm_mday;
    hour = nowTime.tm_hour;
    min = nowTime.tm_min;
    sec = nowTime.tm_sec;
    level = CLEAR_HASH_FLAG(commonContentPtr->level);
    ret = snprintf(outStr, outStrLen, "%04d-%02d-%02d %02d:%02d:%02d %d %c %s: ", year, month,
                   day, hour, min, sec, commonContentPtr->task, g_logLevelInfo[level],
                   HiLogGetModuleName(commonContentPtr->module));
#else
    uint8_t level;
    level = CLEAR_HASH_FLAG(commonContentPtr->level);
    ret = snprintf(outStr, outStrLen, "%d %c %d/%s: ", commonContentPtr->task,
                   g_logLevelInfo[level], commonContentPtr->module,
                   HiLogGetModuleName(commonContentPtr->module));

    (void)outStr;
    (void)outStrLen;
    (void)commonContentPtr;
#endif
    return ret;
}

#ifdef __WIN64
#define CONST_STR_ADDRESS_RANGE 0x1000000
#else
#define CONST_STR_ADDRESS_RANGE 0x10000
#endif

static BOOL IsConstString(uintptr ptr)
{
    uintptr diff = 0;
    uintptr const_str = (uintptr)g_logLevelInfo;
    /* lets assume const string address are in [-1MB, +1MB]*/
    if (ptr > const_str) {
        diff = ptr - const_str;
    } else {
        diff = const_str - ptr;
    }

    if (diff < CONST_STR_ADDRESS_RANGE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static int32 LogValuesFmt(char *desStrPtr, int32 desLen, const HiLogContent *logContentPtr)
{
    int32 i;
    int32 outLen = 0;
    int32 len;
    char fmtStr[SINGLE_FMT_MAX_LEN];
    uint32 valNum = logContentPtr->commonContent.valueNumber;
    const char *fmt = logContentPtr->commonContent.fmt;
    uint32 valueIndex = 0;
    for (i = 0; fmt[i] != 0 && outLen < desLen;) {
        if (fmt[i] != '%') {
            desStrPtr[outLen++] = fmt[i++];
            continue;
        }
        if (fmt[i + 1] == '%') {
            desStrPtr[outLen++] = fmt[i++];
            desStrPtr[outLen++] = fmt[i++];
            continue;
        }
        fmtStr[0] = fmt[i++];
        uint32 t = 1;
        while (fmt[i] != 0 && t < sizeof(fmtStr) - 1) {
            /* %s %ms %-ms %m.ns %-m.ns convert to %p */
            if ((fmt[i] == 's' || fmt[i] == 'S') &&
                (fmt[i - 1] == '%' || (fmt[i - 1] >= '0' && fmt[i - 1] <= '9'))) {
                if (IsConstString(logContentPtr->values[valueIndex])) {
                    fmtStr[1] = 's';
                } else {
                    fmtStr[1] = 'p';
                }
                fmtStr[FMT_CONVERT_TRMINATOR] = 0;
                i++;
                break;
            }
            if ((fmt[i] >= 'a' && fmt[i] <= 'z') || (fmt[i] >= 'A' && fmt[i] <= 'Z')) {
                fmtStr[t++] = fmt[i++];
                fmtStr[t] = 0;
                break;
            }
            fmtStr[t++] = fmt[i++];
        }
        if (valueIndex < valNum) {
            len = snprintf(&desStrPtr[outLen], desLen - outLen, fmtStr,
                           logContentPtr->values[valueIndex]);
            if (len < 0) {
                break;
            }
            outLen += len;
            valueIndex++;
        }
    }

    return outLen;
}

static int32 LogDebugValuesFmt(char *desStrPtr, int32 desLen, const HiLogContent *logContentPtr)
{
    int32 ret = 0;
    switch (logContentPtr->commonContent.valueNumber) {
    case LOG_MULTI_PARA_0:
        // ret = desLen;
        strncpy(desStrPtr, logContentPtr->commonContent.fmt, desLen);
        ret = strlen(desStrPtr);
        break;
    case LOG_MULTI_PARA_1:
        ret = snprintf(desStrPtr, desLen, logContentPtr->commonContent.fmt,
                       logContentPtr->values[0]);
        break;
    case LOG_MULTI_PARA_2:
        ret = snprintf(desStrPtr, desLen, logContentPtr->commonContent.fmt,
                       logContentPtr->values[0], logContentPtr->values[1]);
        break;
    case LOG_MULTI_PARA_3:
        ret = snprintf(desStrPtr, desLen, logContentPtr->commonContent.fmt,
                       logContentPtr->values[0], logContentPtr->values[1],
                       logContentPtr->values[LOG_MULTI_PARA_2]);
        break;
    case LOG_MULTI_PARA_4:
        ret = snprintf(desStrPtr, desLen, logContentPtr->commonContent.fmt,
                       logContentPtr->values[0], logContentPtr->values[1],
                       logContentPtr->values[LOG_MULTI_PARA_2],
                       logContentPtr->values[LOG_MULTI_PARA_3]);
        break;
    case LOG_MULTI_PARA_5:
        ret = snprintf(desStrPtr, desLen, logContentPtr->commonContent.fmt,
                       logContentPtr->values[0], logContentPtr->values[1],
                       logContentPtr->values[LOG_MULTI_PARA_2],
                       logContentPtr->values[LOG_MULTI_PARA_3],
                       logContentPtr->values[LOG_MULTI_PARA_4]);
        break;
    case LOG_MULTI_PARA_MAX:
        ret = snprintf(desStrPtr, desLen, logContentPtr->commonContent.fmt,
                       logContentPtr->values[0], logContentPtr->values[1],
                       logContentPtr->values[LOG_MULTI_PARA_2],
                       logContentPtr->values[LOG_MULTI_PARA_3],
                       logContentPtr->values[LOG_MULTI_PARA_4],
                       logContentPtr->values[LOG_MULTI_PARA_5]);
        break;
    default:
        break;
    }

    if (ret < 0) {
        ret = 0;
    }
    return ret;
}
#if __LP64__
#define FMT_U "%lu"
#elif __WIN64
#define FMT_U "%llu"
#else
#define FMT_U "%u"
#endif
static int32 LogValuesFmtHash(char *desStrPtr, int32 desLen, const HiLogContent *logContentPtr)
{
    int32 outLen = 0;
    uint32 paraNum = logContentPtr->commonContent.valueNumber;
    int32 len = snprintf(&desStrPtr[outLen], desLen - outLen,
                         "hash:%u para:", (uint32)(uintptr)logContentPtr->commonContent.fmt);
    if (len < 0) {
        return len;
    }
    outLen += len;

    for (uint32 i = 0; i < paraNum && i < LOG_MULTI_PARA_MAX; i++) {
        len = snprintf(&desStrPtr[outLen], desLen - outLen, FMT_U " ",
                       logContentPtr->values[i]);
        if (len < 0) {
            return len;
        }
        outLen += len;
    }
    return outLen;
}

void FlushLog(boolean syncFlag)
{
    int8 opt = GETOPTION(g_hiviewConfig.outputOption);
    if (g_logCache.usedSize > 0) {
        if (syncFlag == FALSE) {
            switch (opt) {
            case OUTPUT_OPTION_TEXT_FILE:
                HiviewSendMessage(HIVIEW_SERVICE, HIVIEW_MSG_OUTPUT_LOG_TEXT_FILE, 0);
                break;
            case OUTPUT_OPTION_BIN_FILE:
                HiviewSendMessage(HIVIEW_SERVICE, HIVIEW_MSG_OUTPUT_LOG_BIN_FILE, 0);
                break;
            case OUTPUT_OPTION_FLOW:
                HiviewSendMessage(HIVIEW_SERVICE, HIVIEW_MSG_OUTPUT_LOG_FLOW, 0);
                break;
            default:
                break;
            }
        } else {
            switch (opt) {
#if ENABLE_OHOS_HIEVIEW_LOG_FILE
            case OUTPUT_OPTION_TEXT_FILE:
                OutputLog2TextFile(NULL);
                break;
            case OUTPUT_OPTION_BIN_FILE:
                OutputLog2BinFile(NULL);
                break;
#endif
            case OUTPUT_OPTION_FLOW:
                OutputLogRealtime(NULL);
                break;
            default:
                break;
            }
        }
    }
}

void HiviewRegisterHilogProc(HilogProc func)
{
    g_hilogOutputProc = func;
}

uint32 HiviewGetConfigOption(void)
{
    return GETOPTION(g_hiviewConfig.outputOption);
}

void HiviewUnRegisterHilogProc(HilogProc func)
{
    (void)func;
    if (g_hilogOutputProc != NULL) {
        g_hilogOutputProc = NULL;
    }
}

#if ENABLE_OHOS_HIEVIEW_LOG_FILE
void HiviewRegisterHiLogFileWatcher(FileProc func, const char *path)
{
    if (func == NULL || path == NULL) {
        return;
    }
    RegisterFileWatcher(&g_logFile, func, path);
}

void HiviewUnRegisterHiLogFileWatcher(FileProc func)
{
    if (func == NULL) {
        return;
    }
    UnRegisterFileWatcher(&g_logFile, func);
}

int HiLogFileProcImp(const char *dest, uint8 mode)
{
    FlushLog(TRUE);
    HIVIEW_MutexLock(g_logFlushInfo.mutex);
    int ret = ProcFile(&g_logFile, dest, mode);
    HIVIEW_MutexUnlock(g_logFlushInfo.mutex);
    return ret;
}

void HiLogOutputFileLockImp(void)
{
    HIVIEW_MutexLock(g_outputLogInfo.mutex);
}

void HiLogOutputFileUnLockImp(void)
{
    HIVIEW_MutexUnlock(g_outputLogInfo.mutex);
}
#endif