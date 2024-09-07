

#ifndef LITE_FEATUREIMPL_H
#define LITE_FEATUREIMPL_H

#include "samgr_common.h"
#include "iunknown.h"
#include "feature.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif
    typedef struct FeatureImpl FeatureImpl;

    struct FeatureImpl
    {
        Feature *feature;
        IUnknown *iUnknown;
    };

    inline static BOOL IsInvalidFeature(Feature *feature)
    {
        return (feature == NULL || feature->GetName == NULL || feature->OnInitialize == NULL ||
                feature->OnMessage == NULL || feature->OnStop == NULL);
    }

    inline static BOOL IsInvalidIUnknown(IUnknown *iUnknown)
    {
        return (iUnknown == NULL || iUnknown->QueryInterface == NULL || iUnknown->AddRef == NULL ||
                iUnknown->Release == NULL);
    }
    BOOL SAMGR_AddInterface(FeatureImpl *featureImpl, IUnknown *iUnknown);
    IUnknown *SAMGR_DelInterface(FeatureImpl *featureImpl);
    IUnknown *SAMGR_GetInterface(FeatureImpl *featureImpl);
    BOOL SAMGR_IsNoInterface(FeatureImpl *featureImpl);
    FeatureImpl *FEATURE_CreateInstance(Feature *feature);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // LITE_FEATUREIMPL_H
