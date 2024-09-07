
#include "bootstrap_service.h"
#include <ohos_init.h>
#include "hiview_log.h"
#include "samgr_lite.h"
#include "dfx_log.h"

#define DFX_LOG_MODULE_ID OHOS_MODULE_SAMGR

#define HAS_EVENT_HUB 1

#if HAS_EVENT_HUB
#include "event_hub_service.h"
#endif

#define LOAD_FLAG 0x01
#define TEST_FLAG 0x02
typedef struct Bootstrap
{
    INHERIT_SERVICE;
    Identity identity;
    uint8 flag;
} Bootstrap;
static const char *GetName(Service *service);
static BOOL Initialize(Service *service, Identity identity);
static TaskConfig GetTaskConfig(Service *service);
static BOOL MessageHandle(Service *service, Request *request);

#include "thread_adapter.h"

/* __BootStrapInit can not be static, it is forced to be linked into the target executable
  by linker option -u */
void BootStrapInit(void)
{
    static Bootstrap bootstrap;
    bootstrap.GetName = GetName;
    bootstrap.Initialize = Initialize;
    bootstrap.MessageHandle = MessageHandle;
    bootstrap.GetTaskConfig = GetTaskConfig;
    bootstrap.flag = FALSE;
    SAMGR_GetInstance()->RegisterService((Service *)&bootstrap);
}
SYS_SERVICE_INIT(BootStrapInit);

static const char *GetName(Service *service)
{
    (void)service;
    return BOOTSTRAP_SERVICE;
}

static BOOL Initialize(Service *service, Identity identity)
{
    Bootstrap *bootstrap = (Bootstrap *)service;
    bootstrap->identity = identity;

#if HAS_EVENT_HUB
    pHandle eventHub = NULL;
    eventHub = EventHub_GetInstance();
    EventHub_AddSender(eventHub, &identity, service);
#endif
    return TRUE;
}

static BOOL MessageHandle(Service *service, Request *request)
{
    Bootstrap *bootstrap = (Bootstrap *)service;
#if HAS_EVENT_HUB
    pHandle eventHub = EventHub_GetInstance();
#endif
    switch (request->msgId)
    {
    case BOOT_SYS_COMPLETED:
        if ((bootstrap->flag & LOAD_FLAG) != LOAD_FLAG)
        {
            INIT_APP_CALL(service);
            INIT_APP_CALL(feature);
            bootstrap->flag |= LOAD_FLAG;
        }
        LOGI("core sys completed, app registed, call bootstrap again\n");
#if HAS_EVENT_HUB
        EventHub_SendEvent(eventHub, service, BOOTSTRAP_MSG_BOOT_SYS_COMPLETED);
#endif

        SAMGR_Bootstrap();
        (void)SAMGR_SendResponseByIdentity(&bootstrap->identity, request, NULL);
        break;

    case BOOT_APP_COMPLETED:
        MODULE_CALL(app_run, 0);
        if ((bootstrap->flag & TEST_FLAG) != TEST_FLAG)
        {
            INIT_TEST_CALL();
            bootstrap->flag |= TEST_FLAG;
        }
        LOGI("app/sys service boot completed, app_run/test registed, call bootstrap again\n");
#if HAS_EVENT_HUB
        EventHub_SendEvent(eventHub, service, BOOTSTRAP_MSG_BOOT_APP_COMPLETED);
#endif

        SAMGR_Bootstrap();
        (void)SAMGR_SendResponseByIdentity(&bootstrap->identity, request, NULL);
        break;

    case BOOT_REG_SERVICE:
        MODULE_CALL(dynamic_service, 0);
        LOGI("on reg service completed, dynamic service registed, call bootstrap again\n");
#if HAS_EVENT_HUB
        EventHub_SendEvent(eventHub, service, BOOTSTRAP_MSG_BOOT_REG_COMPLETED);
#endif
        SAMGR_Bootstrap();
        (void)SAMGR_SendResponseByIdentity(&bootstrap->identity, request, NULL);
        break;

    default:
        break;
    }

    return TRUE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    // The bootstrap service uses a stack of 2 KB (0x800) in size and a queue of 20 elements.
    // You can adjust it according to the actual situation.

    // SHARED_TASK use config in DEFAULT_TASK_CFG, don't use the setting here
    TaskConfig config = {LEVEL_HIGH, PRI_NORMAL, 0, 0, SHARED_TASK};
    return config;
}
