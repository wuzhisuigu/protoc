#include "ohos_init.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif
#ifdef LAYER_INIT_IN_CONSTRUCTOR
    int init_call_ohos_end;
    int init_call_device_end;
    int init_call_bsp_end;
    int init_call_core_end;
    int init_call_sys_service_end;
    int init_call_sys_feature_end;
    int init_call_app_service_end;
    int init_call_app_feature_end;
    int init_call_run_end;
    int init_call_app_run_end;
    int init_call_test_end;
    int init_call_dynamic_service_end;

    InitCall init_call_ohos_array[init_call_device_max];
    InitCall init_call_device_array[init_call_device_max];
    InitCall init_call_bsp_array[init_call_bsp_max];
    InitCall init_call_core_array[init_call_core_max];
    InitCall init_call_sys_service_array[init_call_sys_service_max];
    InitCall init_call_sys_feature_array[init_call_sys_feature_max];
    InitCall init_call_app_service_array[init_call_app_service_max];
    InitCall init_call_app_feature_array[init_call_app_feature_max];
    InitCall init_call_app_run_array[init_call_app_run_max];
    InitCall init_call_run_array[init_call_run_max];
    InitCall init_call_test_array[init_call_test_max];
    InitCall init_call_dynamic_service_array[init_call_dynamic_service_max];

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif