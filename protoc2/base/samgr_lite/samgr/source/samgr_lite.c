
#include "message.h"
#include "samgr_common.h"
#include "samgr_lite_inner.h"
#include <string.h>
#include <ohos_errno.h>
#include <dfx_log.h>
#include "memory_adapter.h"
#include "time_adapter.h"
#include "thread_adapter.h"
#include "service_impl.h"
#include "feature_impl.h"
#include "service_registry.h"

#define DFX_LOG_MODULE_ID OHOS_MODULE_SAMGR

/* **************************************************************************************************
 * Samgr Lite public interfaces
 * ************************************************************************************************* */
static BOOL RegisterService(Service *service);
static Service *UnregisterService(const char *name);
static BOOL RegisterFeature(const char *serviceName, Feature *feature);
static Feature *UnregisterFeature(const char *serviceName, const char *featureName);
static BOOL RegisterFeatureApi(const char *serviceName, const char *feature,
                               IUnknown *publicApi);
static IUnknown *UnregisterFeatureApi(const char *serviceName, const char *feature);
static BOOL RegisterDefaultFeatureApi(const char *serviceName, IUnknown *publicApi);
static IUnknown *UnregisterDefaultFeatureApi(const char *serviceName);
static IUnknown *GetDefaultFeatureApi(const char *serviceName);
static IUnknown *GetFeatureApi(const char *serviceName, const char *feature);
static int32 AddSystemCapability(const char *sysCap);
static BOOL HasSystemCapability(const char *sysCap);
static int32 GetSystemAvailableCapabilities(char sysCaps[MAX_SYSCAP_NUM][MAX_SYSCAP_NAME_LEN],
                                            int32 *size);
static BOOL GetServiceId(const char *serviceName, Identity *id);
static BOOL GetFeatureId(const char *serviceName, const char *feature, Identity *id);

/* **************************************************************************************************
 * Samgr Lite location functions
 * ************************************************************************************************* */
static void InitializeAllServices(Vector *services);
static void InitializeSingleService(ServiceImpl *service);
static int32 SendBootRequest(int msgId, uint32 msgValue);
static const char *GetServiceName(const ServiceImpl *serviceImpl);
static short GetUninitializedPos(void);
static void AddTaskPool(ServiceImpl *service, TaskConfig *cfg, const char *name);
static int32 InitCompleted(void);
static void HandleInitRequest(const Request *request, const Response *response);
static SamgrLiteImpl *GetImplement(void);
static ServiceImpl *GetService(const char *name);
static TaskPool *GetSpecifiedTaskPool(TaskConfig *config);
static void Init(void);
/* **************************************************************************************************
 * Samgr Lite location structure and local variable
 * ************************************************************************************************* */
static SamgrLiteImpl g_samgrImpl;

#define TO_NEXT_STATUS(status) (BootStatus)((uint8)(status) | 0x1)

SamgrLite *SAMGR_GetInstance(void)
{
    if (g_samgrImpl.mutex == NULL) {
        Init();
    }
    return &(GetImplement()->vtbl);
}

static SamgrLiteImpl *GetImplement(void)
{
    return &g_samgrImpl;
}

static void Init(void)
{
    WDT_Start(WDG_SAMGR_INIT_TIME);
    g_samgrImpl.vtbl.RegisterService = RegisterService;
    g_samgrImpl.vtbl.UnregisterService = UnregisterService;
    g_samgrImpl.vtbl.RegisterFeature = RegisterFeature;
    g_samgrImpl.vtbl.UnregisterFeature = UnregisterFeature;
    g_samgrImpl.vtbl.RegisterFeatureApi = RegisterFeatureApi;
    g_samgrImpl.vtbl.UnregisterFeatureApi = UnregisterFeatureApi;
    g_samgrImpl.vtbl.RegisterDefaultFeatureApi = RegisterDefaultFeatureApi;
    g_samgrImpl.vtbl.UnregisterDefaultFeatureApi = UnregisterDefaultFeatureApi;
    g_samgrImpl.vtbl.GetDefaultFeatureApi = GetDefaultFeatureApi;
    g_samgrImpl.vtbl.GetFeatureApi = GetFeatureApi;
    g_samgrImpl.vtbl.GetServiceId = GetServiceId;
    g_samgrImpl.vtbl.GetFeatureId = GetFeatureId;
    g_samgrImpl.vtbl.AddSystemCapability = AddSystemCapability;
    g_samgrImpl.vtbl.HasSystemCapability = HasSystemCapability;
    g_samgrImpl.vtbl.GetSystemAvailableCapabilities = GetSystemAvailableCapabilities;
    g_samgrImpl.status = BOOT_SYS;
    g_samgrImpl.services = VECTOR_Make((VECTOR_Key)GetServiceName, (VECTOR_Compare)strcmp);
    g_samgrImpl.mutex = MUTEX_Create();
    (void)memset(g_samgrImpl.sharedPool, 0, sizeof(TaskPool *) * MAX_POOL_NUM);
    WDT_Reset(WDG_SVC_REG_TIME);
    MUTEX_InitGlobalLockValue();
}

void SAMGR_Bootstrap(void)
{
    SamgrLiteImpl *samgr = GetImplement();
    if (samgr->mutex == NULL) {
        LOGI("Samgr is not init, no service!");
        return;
    }
    WDT_Reset(WDG_SVC_BOOT_TIME);
    Vector initServices = VECTOR_Make(NULL, NULL);
    MUTEX_Lock(samgr->mutex);
    samgr->status = TO_NEXT_STATUS(samgr->status);
    int16 size = VECTOR_Size(&(samgr->services));
    int16 i, initSvcSize = 0;
    for (i = 0; i < size; ++i) {
        ServiceImpl *serviceImpl = (ServiceImpl *)VECTOR_At(&(samgr->services), i);
        if (serviceImpl == NULL || serviceImpl->inited != SVC_INIT) {
            continue;
        }
        VECTOR_Add(&initServices, serviceImpl);
        initSvcSize++;
    }

    MUTEX_Unlock(samgr->mutex);
    LOGI_(BOOT_FMT(samgr->status), VECTOR_Size(&initServices));
    // keep running all stages until BOOT_DYNAMIC_WAIT, some stage may be empty, no new services are registered
    // for example, the boot_app state may have no new serivces, but we still need to call app.run zinitcalls
    // BOOT_SYS --> BOOT_SYS_WAIT --> BOOT_APP --> BOOT_APP_WAIT  --> BOOT_DYNAMIC --> BOOT_DYNAMIC_WAIT
    if (!initSvcSize && samgr->status == BOOT_DYNAMIC_WAIT) {
        // InitCompleted() will set samgr->status back to BOOT_DYNAMIC, causing dead loop
        // so break out here
        /* no new service to be inited, return */
        VECTOR_Clear(&initServices);
        return;
    }

    InitializeAllServices(&initServices);
    VECTOR_Clear(&initServices);
    int32 err = InitCompleted();
    if (err != EC_SUCCESS) {
        LOGI("Goto next boot step return code:%d", err);
    }
}

ServiceImpl *SAMGR_GetServiceByID(int16 serviceId)
{
    SamgrLiteImpl *manager = GetImplement();
    MUTEX_Lock(manager->mutex);
    int16 size = VECTOR_Size(&(manager->services));
    if (serviceId < 0 || serviceId > size) {
        MUTEX_Unlock(manager->mutex);
        return NULL;
    }
    ServiceImpl *serviceImpl = (ServiceImpl *)VECTOR_At(&(manager->services), serviceId);
    MUTEX_Unlock(manager->mutex);
    return serviceImpl;
}

static int32 InitCompleted(void)
{
    // Did all services be inited?
    SamgrLiteImpl *manager = GetImplement();
    int16 pos = GetUninitializedPos();
    int16 size = VECTOR_Size(&(manager->services));
    if (pos < size) {
        return EC_SUCCESS;
    }

    MUTEX_Lock(manager->mutex);
    if (manager->status == BOOT_SYS_WAIT) {
        manager->status = BOOT_APP;
        MUTEX_Unlock(manager->mutex);
        LOGI("Initialized all core system services!");
        WDT_Reset(WDG_SVC_REG_TIME);
        return SendBootRequest(BOOT_SYS_COMPLETED, pos);
    }

    if (manager->status == BOOT_APP_WAIT) {
        manager->status = BOOT_DYNAMIC;
        MUTEX_Unlock(manager->mutex);
        LOGI("Initialized all system and application services!");
        WDT_Reset(WDG_SVC_TEST_TIME);
        return SendBootRequest(BOOT_APP_COMPLETED, pos);
    }

    if (manager->status == BOOT_DYNAMIC_WAIT) {
        /* set to BOOT_DYNAMIC  again, need to break the SAMGR_Bootrap() dead loop*/
        manager->status = BOOT_DYNAMIC;
        MUTEX_Unlock(manager->mutex);
        LOGI("dynamic seriver register event!");
        WDT_Reset(WDG_SVC_TEST_TIME);
        return SendBootRequest(BOOT_REG_SERVICE, pos);
    }

    MUTEX_Unlock(manager->mutex);
    WDT_Stop();
    return EC_SUCCESS;
}

static void InitializeAllServices(Vector *services)
{
    SamgrLiteImpl *samgr = GetImplement();
    MUTEX_Lock(samgr->mutex);
    int16 size = VECTOR_Size(services);
    int16 i;
    for (i = 0; i < size; ++i) {
        ServiceImpl *serviceImpl = (ServiceImpl *)VECTOR_At(services, i);
        if (serviceImpl == NULL) {
            continue;
        }

        TaskConfig config = serviceImpl->service->GetTaskConfig(serviceImpl->service);
        const char *name = serviceImpl->service->GetName(serviceImpl->service);
        // allocate queue id to each service
        AddTaskPool(serviceImpl, &config, name);
        LOGI("Init service:%s sid %d, TaskPool:%p", name, serviceImpl->serviceId,
             serviceImpl->taskPool);
    }

    // must luanch initialize callback only when all services have been assigned with queueid
    for (i = 0; i < size; ++i) {
        ServiceImpl *serviceImpl = (ServiceImpl *)VECTOR_At(services, i);
        if (serviceImpl == NULL) {
            continue;
        }

        InitializeSingleService(serviceImpl);
    }

    // SamgrLiteImpl *samgr = GetImplement();
    // MUTEX_Lock(samgr->mutex);
    for (i = 0; i < size; ++i) {
        ServiceImpl *serviceImpl = (ServiceImpl *)VECTOR_At(services, i);
        if (serviceImpl == NULL) {
            continue;
        }
        const char *name = serviceImpl->service->GetName(serviceImpl->service);
        SAMGR_StartTaskPool(serviceImpl->taskPool, name);
    }
    MUTEX_Unlock(samgr->mutex);
}

static BOOL RegisterService(Service *service)
{
    if (IsInvalidService(service)) {
        return FALSE;
    }

    SamgrLiteImpl *samgr = GetImplement();
    MUTEX_Lock(samgr->mutex);
    int16 pos = VECTOR_FindByKey(&(samgr->services), (void *)service->GetName(service));
    if (pos >= 0) {
        MUTEX_Unlock(samgr->mutex);
        return FALSE;
    }

    if (VECTOR_Num(&(samgr->services)) >= MAX_SERVICE_NUM) {
        MUTEX_Unlock(samgr->mutex);
        return FALSE;
    }
    // 是否成功创建了服务的实现对象。
    ServiceImpl *serviceImpl = SAMGR_CreateServiceImpl(service, samgr->status);
    if (serviceImpl == NULL) {
        MUTEX_Unlock(samgr->mutex);
        return FALSE;
    }
    serviceImpl->serviceId = VECTOR_Add(&(samgr->services), serviceImpl);
    MUTEX_Unlock(samgr->mutex);
    if (serviceImpl->serviceId == INVALID_INDEX) {
        MEM_Free(serviceImpl);
        return FALSE;
    }
    return TRUE;
}

static Service *UnregisterService(const char *name)
{
    if (name == NULL) {
        return NULL;
    }

    SamgrLiteImpl *samgr = GetImplement();
    MUTEX_Lock(samgr->mutex);
    Vector *services = &(samgr->services);
    int16 pos = VECTOR_FindByKey(services, (void *)name);
    ServiceImpl *serviceImpl = (ServiceImpl *)VECTOR_At(services, pos);
    if (serviceImpl == NULL || serviceImpl->defaultApi != NULL ||
        VECTOR_Num(&serviceImpl->features) > 0) {
        MUTEX_Unlock(samgr->mutex);
        return NULL;
    }

    VECTOR_Swap(services, pos, NULL);
    // release task pool must be in the lock to keep atomicity of the reference
    SAMGR_ReleaseTaskPool(serviceImpl->taskPool);
    MUTEX_Unlock(samgr->mutex);

    Service *service = serviceImpl->service;
    VECTOR_Clear(&serviceImpl->features);
    MEM_Free(serviceImpl);
    return service;
}

static BOOL RegisterFeature(const char *serviceName, Feature *feature)
{
    if (IsInvalidFeature(feature)) {
        return FALSE;
    }

    ServiceImpl *serviceImpl = GetService(serviceName);
    if (serviceImpl == NULL || serviceImpl->inited != SVC_INIT) {
        return FALSE;
    }

    if (DEFAULT_GetFeature(serviceImpl, feature->GetName(feature)) != NULL) {
        return FALSE;
    }

    int16 featureId = DEFAULT_AddFeature(serviceImpl, feature);
    if (featureId < 0) {
        return FALSE;
    }
    return TRUE;
}

static Feature *UnregisterFeature(const char *serviceName, const char *featureName)
{
    ServiceImpl *serviceImpl = GetService(serviceName);
    if (serviceImpl == NULL) {
        return NULL;
    }
    return DEFAULT_DeleteFeature(serviceImpl, featureName);
}

static BOOL RegisterFeatureApi(const char *serviceName, const char *feature,
                               IUnknown *publicApi)
{
    if (IsInvalidIUnknown(publicApi)) {
        return FALSE;
    }

    ServiceImpl *serviceImpl = GetService(serviceName);
    if (serviceImpl == NULL) {
        return FALSE;
    }

    if (feature == NULL) {
        if (serviceImpl->defaultApi != NULL) {
            return FALSE;
        }
        serviceImpl->defaultApi = publicApi;
        return TRUE;
    }

    FeatureImpl *featureImpl = DEFAULT_GetFeature(serviceImpl, feature);
    if (featureImpl == NULL) {
        return FALSE;
    }
    return SAMGR_AddInterface(featureImpl, publicApi);
}

static IUnknown *UnregisterFeatureApi(const char *serviceName, const char *feature)
{
    ServiceImpl *serviceImpl = GetService(serviceName);
    if (serviceImpl == NULL) {
        return NULL;
    }

    if (feature == NULL) {
        IUnknown *iUnknown = serviceImpl->defaultApi;
        serviceImpl->defaultApi = NULL;
        return iUnknown;
    }

    return SAMGR_DelInterface(DEFAULT_GetFeature(serviceImpl, feature));
}

static BOOL RegisterDefaultFeatureApi(const char *serviceName, IUnknown *publicApi)
{
    return RegisterFeatureApi(serviceName, NULL, publicApi);
}

static IUnknown *UnregisterDefaultFeatureApi(const char *serviceName)
{
    return UnregisterFeatureApi(serviceName, NULL);
}

static IUnknown *GetDefaultFeatureApi(const char *serviceName)
{
    return GetFeatureApi(serviceName, NULL);
}

static int32 AddSystemCapability(const char *sysCap)
{
    if (sysCap == NULL || strlen(sysCap) == 0 || strlen(sysCap) > MAX_SYSCAP_NAME_LEN) {
        return EC_INVALID;
    }
    return SAMGR_RegisterSystemCapabilityApi(sysCap, TRUE);
}

static BOOL HasSystemCapability(const char *sysCap)
{
    if (sysCap == NULL || strlen(sysCap) == 0 || strlen(sysCap) > MAX_SYSCAP_NAME_LEN) {
        return FALSE;
    }
    return SAMGR_QuerySystemCapabilityApi(sysCap);
}

static int32 GetSystemAvailableCapabilities(char sysCaps[MAX_SYSCAP_NUM][MAX_SYSCAP_NAME_LEN],
                                            int32 *sysCapNum)
{
    if (sysCaps == NULL || sysCapNum == NULL) {
        return EC_INVALID;
    }
    return SAMGR_GetSystemCapabilitiesApi(sysCaps, sysCapNum);
}

static IUnknown *GetFeatureApi(const char *serviceName, const char *feature)
{
    ServiceImpl *serviceImpl = GetService(serviceName);
    if (serviceImpl == NULL) {
        return SAMGR_FindServiceApi(serviceName, feature);
    }

    FeatureImpl *featureImpl = DEFAULT_GetFeature(serviceImpl, feature);
    if (featureImpl == NULL && feature == NULL) {
        return serviceImpl->defaultApi;
    }

    return SAMGR_GetInterface(featureImpl);
}
static BOOL GetServiceId(const char *serviceName, Identity *id)
{
    ServiceImpl *service = GetService(serviceName);
    if (!service) {
        return FALSE;
    }
    id->serviceId = service->serviceId;
    id->featureId = INVALID_INDEX;
    id->queueId = (service->taskPool != NULL) ? service->taskPool->queueId : NULL;
    return TRUE;
}
static BOOL GetFeatureId(const char *serviceName, const char *feature, Identity *id)
{
    ServiceImpl *service = GetService(serviceName);
    if (!service) {
        return FALSE;
    }
    id->serviceId = service->serviceId;
    // id->featureId = INVALID_INDEX;
    id->queueId = (service->taskPool != NULL) ? service->taskPool->queueId : NULL;

    int16 pos = VECTOR_FindByKey(&(service->features), (void *)feature);
    FeatureImpl *featureImpl = (FeatureImpl *)VECTOR_At(&(service->features), pos);
    if (featureImpl == NULL) {
        return FALSE;
    }
    id->featureId = pos;
    return TRUE;
}

static short GetUninitializedPos()
{
    SamgrLiteImpl *manager = GetImplement();
    MUTEX_Lock(manager->mutex);
    int16 size = VECTOR_Size(&(manager->services));
    int16 i;
    for (i = 0; i < size; ++i) {
        ServiceImpl *service = (ServiceImpl *)VECTOR_At(&(manager->services), i);
        if (service == NULL) {
            continue;
        }

        if (service->inited == SVC_INIT) {
            MUTEX_Unlock(manager->mutex);
            return i;
        }
    }
    MUTEX_Unlock(manager->mutex);
    return size;
}

static const char *GetServiceName(const ServiceImpl *serviceImpl)
{
    if (serviceImpl == NULL) {
        return NULL;
    }
    return serviceImpl->service->GetName(serviceImpl->service);
}

static void AddTaskPool(ServiceImpl *service, TaskConfig *cfg, const char *name)
{
    if (service->taskPool != NULL) {
        return;
    }

    if (cfg->priority < PRI_LOW || cfg->priority >= PRI_BUTT) {
        LOGE("The %s service pri(%d) is out of range!", name, cfg->priority);
        cfg->priority = PRI_LOW;
    }

    switch (cfg->taskFlags) {
    case SHARED_TASK: {
        int pos = (int)cfg->priority / PROPERTY_STEP;
        SamgrLiteImpl *samgr = GetImplement();
        if (samgr->sharedPool[pos] == NULL) {
            TaskConfig shareCfg = DEFAULT_TASK_CFG(pos);
            samgr->sharedPool[pos] = SAMGR_CreateFixedTaskPool(&shareCfg, name, DEFAULT_SIZE);
        }
        service->taskPool = samgr->sharedPool[pos];
        if (SAMGR_ReferenceTaskPool(service->taskPool) == NULL) {
            LOGE("shared task:%p pri:%d ref is full", service->taskPool, cfg->priority);
            samgr->sharedPool[pos] = NULL;
        }
    } break;

    case SPECIFIED_TASK:
        service->taskPool = GetSpecifiedTaskPool(cfg);
        if (service->taskPool != NULL) {
            break;
        }
        // fallthrough
    case SINGLE_TASK:
        service->taskPool = SAMGR_CreateFixedTaskPool(cfg, name, SINGLE_SIZE);
        break;
    default:
        break;
    }

    if (service->taskPool == NULL) {
        LOGE("Service<name:%s, flag:%d> create taskPool failed!", name, cfg->taskFlags);
    }
}

static ServiceImpl *GetService(const char *name)
{
    if (name == NULL) {
        return NULL;
    }

    SamgrLiteImpl *manager = GetImplement();
    MUTEX_Lock(manager->mutex);
    Vector *services = &(manager->services);
    short pos = VECTOR_FindByKey(services, (void *)name);
    if (pos < 0) {
        MUTEX_Unlock(manager->mutex);
        return NULL;
    }
    ServiceImpl *serviceImpl = (ServiceImpl *)VECTOR_At(services, pos);
    MUTEX_Unlock(manager->mutex);
    return serviceImpl;
}

static int32 SendBootRequest(int msgId, uint32 msgValue)
{
    Identity id = DEFAULT_GetFeatureId(GetService(BOOTSTRAP_SERVICE), NULL);
    Request request = { msgId, 0, NULL, msgValue };
    // return SAMGR_SendRequest(&id, &request, (Handler)SAMGR_Bootstrap);
    // change to run SAMGR_Bootstrap in bootstrap service
    return SAMGR_SendRequest(&id, &request, (Handler)NULL);
}

static void HandleInitRequest(const Request *request, const Response *response)
{
    ServiceImpl *serviceImpl = (ServiceImpl *)request->data;
    if (serviceImpl == NULL) {
        LOGE("Init service Request:<%d,%d>, Response:<%p,%d>!", request->msgId,
             request->msgValue, response->data, response->len);
        return;
    }
    uint32 lastTime = serviceImpl->ops.timestamp;
    uint32 nowTime = serviceImpl->ops.timestamp;

    DEFAULT_Initialize(serviceImpl);

    nowTime = SAMGR_GetProcessTime();
    serviceImpl->ops.timestamp = nowTime;
    serviceImpl->inited = SVC_IDLE;
    uint32 diff = nowTime - lastTime;
    LOGI("Init service %s <time: %ums> success!",
         serviceImpl->service->GetName(serviceImpl->service), diff);
    int32 err = InitCompleted();
    if (err != EC_SUCCESS) {
        LOGI("Goto next boot step return code:%d", err);
    }
}

static void InitializeSingleService(ServiceImpl *service)
{
    // service is guaranteed not to be NULL by the caller; and it's a static function, won't have external call.
    if (service->taskPool == NULL) {
        DEFAULT_Initialize(service);
        return;
    }

    Identity identity = { service->serviceId, INVALID_INDEX, service->taskPool->queueId };
    Request request = { 0, 0, service, 0 };
    uint32 *ref = NULL;
    (void)SAMGR_SendSharedDirectRequest(&identity, &request, NULL, &ref, HandleInitRequest);
}

static TaskPool *GetSpecifiedTaskPool(TaskConfig *config)
{
    SamgrLiteImpl *samgr = GetImplement();
    Vector *services = &(samgr->services);
    MUTEX_Lock(samgr->mutex);
    int16 serviceNum = VECTOR_Size(services);
    int i;
    for (i = 0; i < serviceNum; ++i) {
        ServiceImpl *impl = VECTOR_At(services, i);
        if (impl == NULL) {
            continue;
        }

        TaskConfig cfg = impl->service->GetTaskConfig(impl->service);
        if (memcmp(&cfg, config, sizeof(TaskConfig)) != 0) {
            continue;
        }

        if (impl->taskPool == NULL) {
            break;
        }

        TaskPool *taskPool = SAMGR_ReferenceTaskPool(impl->taskPool);
        if (taskPool != NULL) {
            MUTEX_Unlock(samgr->mutex);
            return taskPool;
        }
    }
    MUTEX_Unlock(samgr->mutex);
    return NULL;
}
