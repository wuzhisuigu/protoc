
#include "feature.h"
#include "feature_impl.h"
#include "memory_adapter.h"

BOOL SAMGR_AddInterface(FeatureImpl *featureImpl, IUnknown *iUnknown)
{
    if (featureImpl == NULL || iUnknown == NULL || featureImpl->iUnknown != NULL)
    {
        return FALSE;
    }

    featureImpl->iUnknown = iUnknown;
    return TRUE;
}

IUnknown *SAMGR_DelInterface(FeatureImpl *featureImpl)
{
    if (featureImpl == NULL)
    {
        return NULL;
    }
    IUnknown *iUnknown = featureImpl->iUnknown;
    featureImpl->iUnknown = NULL;
    return iUnknown;
}

IUnknown *SAMGR_GetInterface(FeatureImpl *featureImpl)
{
    if (featureImpl == NULL)
    {
        return NULL;
    }
    return featureImpl->iUnknown;
}

BOOL SAMGR_IsNoInterface(FeatureImpl *featureImpl)
{
    return (BOOL)(featureImpl == NULL || featureImpl->iUnknown == NULL);
}

FeatureImpl *FEATURE_CreateInstance(Feature *feature)
{
    if (feature == NULL)
    {
        return NULL;
    }
    FeatureImpl *featureImpl = (FeatureImpl *)MEM_Malloc(sizeof(FeatureImpl));
    if (featureImpl == NULL)
    {
        return NULL;
    }
    featureImpl->feature = feature;
    featureImpl->iUnknown = NULL;
    return featureImpl;
}