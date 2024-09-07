
#ifndef DFX_LOG_H
#define DFX_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hiview_log.h"

#undef MOD_DEF
#define MOD_DEF(name) OHOS_MODULE_##name,

    enum OhosModuleId
    {
#include "ohos_modules.def"
    };

#define LOGD(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_DEBUG, FUN_ARG_NUM(FUN_ARG_I, ##__VA_ARGS__), "[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#define LOGW(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_WARN, FUN_ARG_NUM(FUN_ARG_I, ##__VA_ARGS__), "[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#define LOGE(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_ERROR, FUN_ARG_NUM(FUN_ARG_I, ##__VA_ARGS__), "[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#define LOGI(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_INFO, FUN_ARG_NUM(FUN_ARG_I, ##__VA_ARGS__), "[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)
#define LOGF(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_FATAL, FUN_ARG_NUM(FUN_ARG_I, ##__VA_ARGS__), "[%s] " fmt, __FUNCTION__, ##__VA_ARGS__)

#define LOGD_(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_DEBUG, FUN_ARG_NUM(__VA_ARGS__), fmt, ##__VA_ARGS__)
#define LOGW_(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_WARN, FUN_ARG_NUM(__VA_ARGS__), fmt, ##__VA_ARGS__)
#define LOGE_(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_ERROR, FUN_ARG_NUM(__VA_ARGS__), fmt, ##__VA_ARGS__)
#define LOGI_(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_INFO, FUN_ARG_NUM(__VA_ARGS__), fmt, ##__VA_ARGS__)
#define LOGF_(fmt, ...) HiLogPrintf(DFX_LOG_MODULE_ID, HILOG_LV_FATAL, FUN_ARG_NUM(__VA_ARGS__), fmt, ##__VA_ARGS__)
// #define DFX_LOG_MODULE_ID OHOS_MODULE_DFX
#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef DFX_LOG_H */