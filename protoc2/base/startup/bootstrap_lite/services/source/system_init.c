
#include "core_main.h"
#include <ohos_init.h>
#include <samgr_lite.h>

#include "thread_adapter.h"

static int app_ok = 0;
static void test_init()
{
    app_ok = TRUE;
}

TEST_INIT(test_init);

void OHOS_SystemInit(void)
{
    MODULE_INIT(bsp);
    MODULE_INIT(device);
    MODULE_INIT(core);
    SYS_INIT(service);
    SYS_INIT(feature);
    MODULE_INIT(run);
    SAMGR_Bootstrap();
}

void OHOS_SystemWaitBootState(int state)
{
    do
    {
        THREAD_Sleep(10);
    } while (app_ok == FALSE);
    (void)state;
}

#define OHOS_INIT(func) LAYER_INITCALL_DEF(func, ohos, "ohos")
OHOS_INIT(OHOS_SystemInit);
