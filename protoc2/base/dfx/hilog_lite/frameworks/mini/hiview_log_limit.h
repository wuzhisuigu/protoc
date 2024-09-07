

#ifndef HOS_LITE_HIVIEW_LOG_LIMIT_H
#define HOS_LITE_HIVIEW_LOG_LIMIT_H

#include "ohos_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* End of #ifdef __cplusplus */

    /* The duration of the check is fixed at 60s. */
    typedef struct
    {
        uint8 maxNum; /* the max num limit of log line */
        uint8 logNum;
        uint16 baseTime;
    } HiLogLimitRule;

    void InitLogLimit(void);
    boolean LogIsLimited(uint8 module);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef HOS_LITE_HIVIEW_LOG_LIMIT_H */