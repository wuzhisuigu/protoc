#include "ohos_init.h"
#include "hiview_def.h"
#include "hiview_util.h"
#include "hiview_config.h"
#include "hiview_log.h"
#include "hiview_log_limit.h"

#define LOG_LIMIT_CHECK_DURATION 60 /* seconds */
#define LOG_LIMIT_LEVEL1 6          /* Maximum number of log records in a check period. */
#define LOG_LIMIT_LEVEL2 30
#define LOG_LIMIT_LEVEL3 60
#define LOG_LIMIT_LEVEL4 120
#ifndef LOG_LIMIT_DEFAULT
#define LOG_LIMIT_DEFAULT LOG_LIMIT_LEVEL2
#endif

static HiLogLimitRule *g_hilogLimitList = NULL;
static void SetLimitThreshold(uint8 module, uint8 v);

void InitLogLimit(void)
{
    if (g_hilogLimitList == NULL)
    {
        g_hilogLimitList = (HiLogLimitRule *)HIVIEW_MemAlloc(
            MEM_POOL_HIVIEW_ID, sizeof(HiLogLimitRule) * HILOG_MODULE_MAX);
        if (g_hilogLimitList == NULL)
        {
            return;
        }
    }

    uint16 i;
    HiLogLimitRule *pRule = NULL;
    for (i = 0; i < HILOG_MODULE_MAX; i++)
    {
        pRule = g_hilogLimitList + i;
        pRule->maxNum = LOG_LIMIT_DEFAULT;
        pRule->logNum = 0;
        pRule->baseTime = 0;
    }
    SetLimitThreshold(HILOG_MODULE_HIVIEW, LOG_LIMIT_LEVEL3);
    SetLimitThreshold(HILOG_MODULE_APP, LOG_LIMIT_LEVEL2);
    HILOG_DEBUG(HILOG_MODULE_HIVIEW, "log limit init success.");
}

boolean LogIsLimited(uint8 module)
{
    if (GETOPTION(g_hiviewConfig.outputOption) == OUTPUT_OPTION_DEBUG)
    {
        return FALSE;
    }
    /* covert ms to sec by dividing 1000, and integer overflow can be accepted */
    uint16 curTime = (uint16)(HIVIEW_GetCurrentTime() / MS_PER_SECOND);
    uint8 logNum;
    uint16 baseTime;
    HiLogLimitRule *pLimitRule = NULL;

    if (module >= HILOG_MODULE_MAX)
    {
        return TRUE;
    }
    if (g_hilogLimitList == NULL)
    {
        return FALSE;
    }

    pLimitRule = g_hilogLimitList + module;
    if (pLimitRule->baseTime == 0)
    {
        pLimitRule->baseTime = (uint16)(HIVIEW_GetCurrentTime() / MS_PER_SECOND);
    }
    pLimitRule->logNum++;
    logNum = pLimitRule->logNum;
    baseTime = pLimitRule->baseTime;
    if ((curTime < baseTime) || ((curTime - baseTime) >= LOG_LIMIT_CHECK_DURATION))
    {
        pLimitRule->baseTime = curTime;
        pLimitRule->logNum = 0;
        if (logNum > pLimitRule->maxNum)
        {
            HILOG_WARN(module, "log limit, drop lines %u", logNum - pLimitRule->maxNum);
        }
    }
    else
    {
        if (logNum > pLimitRule->maxNum)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void SetLimitThreshold(uint8 module, uint8 v)
{
    if (g_hilogLimitList == NULL)
    {
        return;
    }

    HiLogLimitRule *pRule = g_hilogLimitList + module;
    pRule->maxNum = v;
    pRule->logNum = 0;
    pRule->baseTime = 0;
}
