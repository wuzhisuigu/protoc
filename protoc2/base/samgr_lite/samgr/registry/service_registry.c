
#include "service_registry.h"
#include <ohos_errno.h>

#ifdef __WIN64
#define REG_ATTR
#else
#define REG_ATTR __attribute__((weak))
#endif

int REG_ATTR SAMGR_RegisterServiceApi(const char *service, const char *feature,
                                      const Identity *identity, IUnknown *iUnknown)
{
    (void)service;
    (void)feature;
    (void)iUnknown;
    (void)identity;
    return EC_FAILURE;
}

IUnknown *REG_ATTR SAMGR_FindServiceApi(const char *service, const char *feature)
{
    (void)service;
    (void)feature;
    return NULL;
}

int32 REG_ATTR SAMGR_RegisterSystemCapabilityApi(const char *sysCap, BOOL isReg)
{
    (void)sysCap;
    (void)isReg;
    return EC_FAILURE;
}

BOOL REG_ATTR SAMGR_QuerySystemCapabilityApi(const char *sysCap)
{
    (void)sysCap;
    return FALSE;
}

int32 REG_ATTR SAMGR_GetSystemCapabilitiesApi(char sysCaps[MAX_SYSCAP_NUM][MAX_SYSCAP_NAME_LEN],
                                              int32 *size)
{
    (void)sysCaps;
    (void)size;
    return EC_FAILURE;
}