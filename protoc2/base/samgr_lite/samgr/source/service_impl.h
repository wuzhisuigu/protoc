

#ifndef LITE_SERVICEIMPL_H
#define LITE_SERVICEIMPL_H

#include "samgr_common.h"
#include "iunknown.h"
#include "service.h"
#include "feature_impl.h"
#include "task_manager.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

    enum InitStatus
    {
        SVC_INIT = 0,
        SVC_IDLE,
        SVC_BUSY,
    };
#ifndef MAX_FEATURE_NUM
#define MAX_FEATURE_NUM 0x70
#endif
    typedef struct Operations Operations;
    struct Operations
    {
        uint8 abnormal;
        uint8 step;
        uint16 messages;
        uint32 timestamp;
    };
    typedef struct ServiceImpl ServiceImpl;
    struct ServiceImpl
    {
        Service *service;
        IUnknown *defaultApi;
        TaskPool *taskPool;
        Vector features;
        int16 serviceId;
        uint8 inited;
        Operations ops;
    };

    inline static BOOL IsInvalidService(Service *service)
    {
        return (service == NULL || service->GetName == NULL || service->Initialize == NULL ||
                service->GetTaskConfig == NULL || service->MessageHandle == NULL);
    }

    int16 DEFAULT_AddFeature(ServiceImpl *serviceImpl, Feature *feature);
    void DEFAULT_Initialize(ServiceImpl *impl);
    void DEFAULT_MessageHandle(ServiceImpl *serviceImpl, const Identity *identity, Request *msg);
    void DEFAULT_StopService(ServiceImpl *service);
    FeatureImpl *DEFAULT_GetFeature(ServiceImpl *serviceImpl, const char *featureName);
    Feature *DEFAULT_DeleteFeature(ServiceImpl *serviceImpl, const char *featureName);
    Identity DEFAULT_GetFeatureId(ServiceImpl *serviceImpl, const char *feature);
    ServiceImpl *SAMGR_CreateServiceImpl(Service *service, uint8 step);
    ServiceImpl *SAMGR_GetServiceByID(int16 serviceId);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_SERVICEIMPL_H
