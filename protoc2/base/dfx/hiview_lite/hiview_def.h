#ifndef HOS_LITE_HIVIEW_DEF_H
#define HOS_LITE_HIVIEW_DEF_H

#include "ohos_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

#define HIVIEW_SERVICE "hiview"
#define LOG_INFO_HEAD 0xEC
#define EVENT_INFO_HEAD 0xEA
#define MS_PER_SECOND 1000
#define SECONDS_PER_DAY 86400
#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_MINUTE 60
#ifdef __WIN64
#define LOG_FMT_MAX_LEN 256
#else
#define LOG_FMT_MAX_LEN 128
#endif
#define LOG_CONTENT_MAX_LEN 96
#define LOG_MODULE_NAME_LEN 16
#define LOG_TIME_STR_SIZE 10
#define TAIL_LINE_BREAK 3
#define READ_FILE_BUF_MAX_SIZE 1024

#pragma pack(1)
    typedef struct
    {
        uint8 id;
        const char *name; /* LOG_MODULE_NAME_LEN */
    } HiLogModuleInfo;
#pragma pack()

    typedef struct
    {
        uint8 last : 1; /* Indicates whether the last parameter. */
        uint8 id : 4;
        uint8 len : 3;
    } HiEventTag;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef HOS_LITE_HIVIEW_DEF_H */
