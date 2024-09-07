
#ifndef LITE_SERVICE_REGISTRY_H
#define LITE_SERVICE_REGISTRY_H

#include <iunknown.h>
#include <message.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif
#define MAX_SYSCAP_NUM 512
#define MAX_SYSCAP_NAME_LEN 64
    int SAMGR_RegisterServiceApi(const char *service, const char *feature, const Identity *identity, IUnknown *iUnknown);
    IUnknown *SAMGR_FindServiceApi(const char *service, const char *feature);
    int32 SAMGR_RegisterSystemCapabilityApi(const char *sysCap, BOOL isReg);
    BOOL SAMGR_QuerySystemCapabilityApi(const char *sysCap);
    int32 SAMGR_GetSystemCapabilitiesApi(char sysCaps[MAX_SYSCAP_NUM][MAX_SYSCAP_NAME_LEN], int32 *size);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_SERVICE_REGISTRY_H
