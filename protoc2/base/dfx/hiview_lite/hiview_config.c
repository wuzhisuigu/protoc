

#include "ohos_init.h"
#include "hiview_config.h"

HiviewConfig g_hiviewConfig = {
    .outputOption = OUTPUT_OPTION, // print via hiewview service
    // .outputOption = OUTPUT_OPTION_DEBUG, // print to uart directly
    /* Control log output level. HILOG_LV_XXX, default is HILOG_LV_DEBUG */
    .level = OUTPUT_LEVEL,
    .logSwitch = HILOG_LITE_SWITCH,
    .dumpSwitch = DUMP_LITE_SWITCH,
    .eventSwitch = HIEVENT_LITE_SWITCH,
    .logFileSizeMax = HIVIEW_LOG_FILE_SIZE,
};

void HiviewConfigInit(void)
{
    g_hiviewConfig.hiviewInited = FALSE;
    g_hiviewConfig.logOutputModule = (uint64_t)0xFFFFFFFF;
    g_hiviewConfig.writeFailureCount = 0;
}
CORE_INIT_PRI(HiviewConfigInit, 0);
