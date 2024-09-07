

/**
 * @addtogroup Init OHOS Init
 * @{
 *
 * @brief Provides the entries for initializing and starting services and features.
 *
 * This module provides the entries for initializing services and features during service
 * development. \n
 * Services and features are initialized in the following sequence: core phase, core system
 * service, core system feature, system startup, system service, system feature, application-layer
 * service, and application-layer feature. \n
 *
 * @since 1.0
 * @version 1.0
 */

/**
 * @file ohos_init.h
 *
 * @brief Provides the entries for initializing and starting services and features.
 *
 * This file provides the entries for initializing services and features during service
 * development. \n
 *
 * @since 1.0
 * @version 1.0
 */

#ifndef OHOS_LAYER_INIT_H
#define OHOS_LAYER_INIT_H

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif
    typedef void (*InitCall)(void);

#define USED_ATTR __attribute__((used))

#if defined(__APPLE__) || defined(__riscv) || defined(__WIN64)

#define LAYER_INIT_IN_CONSTRUCTOR
#endif

#ifdef LAYER_INIT_IN_CONSTRUCTOR
#define LAYER_INIT_LEVEL_0 0
#define LAYER_INIT_LEVEL_1 1
#define LAYER_INIT_LEVEL_2 2
#define LAYER_INIT_LEVEL_3 3
#define LAYER_INIT_LEVEL_4 4
#define CTOR_VALUE_ohos 101
#define CTOR_VALUE_bsp 102
#define CTOR_VALUE_device 105
#define CTOR_VALUE_core 110
#define CTOR_VALUE_sys_service 120
#define CTOR_VALUE_sys_feature 130
#define CTOR_VALUE_app_service 140
#define CTOR_VALUE_app_feature 150
#define CTOR_VALUE_run 700
#define CTOR_VALUE_test 720
#define CTOR_VALUE_dynamic_service 740

#define init_call_ohos_max 2
#define init_call_device_max 10
#define init_call_bsp_max 10
#define init_call_core_max 10
#define init_call_sys_service_max 20
#define init_call_sys_feature_max 20
#define init_call_app_service_max 20
#define init_call_app_feature_max 20
#define init_call_app_run_max 10
#define init_call_run_max 10
#define init_call_test_max 10
#define init_call_dynamic_service_max 10

    extern int init_call_ohos_end;
    extern int init_call_device_end;
    extern int init_call_bsp_end;
    extern int init_call_core_end;
    extern int init_call_sys_service_end;
    extern int init_call_sys_feature_end;
    extern int init_call_app_service_end;
    extern int init_call_app_feature_end;
    extern int init_call_app_run_end;
    extern int init_call_run_end;
    extern int init_call_test_end;
    extern int init_call_dynamic_service_end;

    extern InitCall init_call_ohos_array[init_call_device_max];
    extern InitCall init_call_device_array[init_call_device_max];
    extern InitCall init_call_bsp_array[init_call_bsp_max];
    extern InitCall init_call_core_array[init_call_core_max];
    extern InitCall init_call_sys_service_array[init_call_sys_service_max];
    extern InitCall init_call_sys_feature_array[init_call_sys_feature_max];
    extern InitCall init_call_app_service_array[init_call_app_service_max];
    extern InitCall init_call_app_feature_array[init_call_app_feature_max];
    extern InitCall init_call_app_run_array[init_call_app_run_max];
    extern InitCall init_call_run_array[init_call_run_max];
    extern InitCall init_call_test_array[init_call_test_max];
    extern InitCall init_call_dynamic_service_array[init_call_dynamic_service_max];

#define LAYER_INITCALL(func, layer, clayer, priority)                                                    \
    static __attribute__((constructor(CTOR_VALUE_##layer +                                               \
                                      LAYER_INIT_LEVEL_##priority))) void BOOT_##layer##priority##func() \
    {                                                                                                    \
        if (init_call_##layer##_end < init_call_##layer##_max)                                           \
        {                                                                                                \
            init_call_##layer##_array[init_call_##layer##_end++] = func;                                 \
        }                                                                                                \
    }

#else // not LAYER_INIT_IN_CONSTRUCTOR
#define LAYER_INITCALL(func, layer, clayer, priority)            \
    static const InitCall USED_ATTR __zinitcall_##layer##_##func \
        __attribute__((section(".zinitcall." clayer #priority ".init"))) = func
#endif
// Default priority is 2, priority range is [0, 4]
#define LAYER_INITCALL_DEF(func, layer, clayer) LAYER_INITCALL(func, layer, clayer, 2)

/**
 * @brief Identifies the entry for initializing and starting a core phase by the priority 2.
 *
 * This macro is used when Samgr is initialized and started. \n
 * This macro is used to identify the entry called at the priority 2 of the core phase of
 * the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a core phase.
 * The type is void (*)(void).
 */
#define CORE_INIT(func) LAYER_INITCALL_DEF(func, core, "core")
/**
 * @brief Identifies the entry for initializing and starting a core phase by the specified
 * priority.
 *
 * This macro is used when Samgr is initialized and started. \n
 * This macro is used to identify the entry called at the specified priority of the core phase of
 * the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a core phase.
 * The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the core phase.
 * The value range is [0,5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define CORE_INIT_PRI(func, priority) LAYER_INITCALL(func, core, "core", priority)

/**
 * @brief Identifies the entry for initializing and starting a core system service by the
 * priority 2.
 *
 * This macro is used to identify the entry called at the priority 2 in the core system
 * service phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a core system service.
 * The type is void (*)(void).
 */
#define SYS_SERVICE_INIT(func) LAYER_INITCALL_DEF(func, sys_service, "sys.service")
/**
 * @brief Identifies the entry for initializing and starting a core system service by the
 * specified priority.
 *
 * This macro is used to identify the entry called at the specified priority in the core system
 * service phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a core system service.
 * The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the core system service in the
 * startup phase. The value range is [0,5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define SYS_SERVICE_INIT_PRI(func, priority) \
    LAYER_INITCALL(func, sys_service, "sys.service", priority)

/**
 * @brief Identifies the entry for initializing and starting a core system feature by the
 * priority 2.
 *
 * This macro is used to identify the entry called at the priority 2 in the core system
 * feature phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a core system service.
 * The type is void (*)(void).
 */
#define SYS_FEATURE_INIT(func) LAYER_INITCALL_DEF(func, sys_feature, "sys.feature")
/**
 * @brief Identifies the entry for initializing and starting a core system feature by the
 * specified priority.
 *
 * This macro is used to identify the entry called at the specified priority in the core system
 * feature phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a core system feature.
 * The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the core system feature phase.
 * The value range is [0, 5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define SYS_FEATURE_INIT_PRI(func, priority) \
    LAYER_INITCALL(func, sys_feature, "sys.feature", priority)

/**
 * @brief Identifies the entry for initializing and starting a system running phase by the
 * priority 2.
 *
 * This macro is used to identify the entry called at the priority 2 in the system startup
 * phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a system running phase.
 * The type is void (*)(void).
 */
#define SYS_RUN(func) LAYER_INITCALL_DEF(func, run, "run")
/**
 * @brief Identifies the entry for initializing and starting a system running phase by the
 * specified priority.
 *
 * This macro is used to identify the entry called at the specified priority in the system startup
 * phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a system running phase.
 * The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the system startup phase.
 * The value range is [0, 5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define SYS_RUN_PRI(func, priority) LAYER_INITCALL(func, run, "run", priority)

/**
 * @brief Identifies the entry for initializing and starting a system service by the priority 2.
 *
 * This macro is used to identify the entry called at the priority 2 in the system service
 * phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a system service.
 * The type is void (*)(void).
 */
#define SYSEX_SERVICE_INIT(func) LAYER_INITCALL_DEF(func, app_service, "app.service")
/**
 * @brief Identifies the entry for initializing and starting a system service by the specified
 * priority.
 *
 * This macro is used to identify the entry called at the specified priority of the system service
 * phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a system service.
 * The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the system service phase.
 * The value range is [0,5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define SYSEX_SERVICE_INIT_PRI(func, priority) \
    LAYER_INITCALL(func, app_service, "app.service", priority)

/**
 * @brief Identifies the entry for initializing and starting a system feature by the priority 2.
 *
 * This macro is used to identify the entry called at the priority 2 of the system feature
 * phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a system feature.
 * The type is void (*)(void).
 */
#define SYSEX_FEATURE_INIT(func) LAYER_INITCALL_DEF(func, app_feature, "app.feature")
/**
 * @brief Identifies the entry for initializing and starting a system feature by the specified
 * priority.
 *
 * This macro is used to identify the entry called at the specified priority of the system feature
 * phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting a system feature.
 * The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the system feature phase.
 * The value range is [0,5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define SYSEX_FEATURE_INIT_PRI(func, priority) \
    LAYER_INITCALL(func, app_feature, "app.feature", priority)

/**
 * @brief Identifies the entry for initializing and starting an application-layer service by the
 * priority 2.
 *
 * This macro is used to identify the entry called at the priority 2 of the application-layer
 * service phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting an application-layer
 * service. The type is void (*)(void).
 */
#define APP_SERVICE_INIT(func) LAYER_INITCALL_DEF(func, app_service, "app.service")
/**
 * @brief Identifies the entry for initializing and starting an application-layer service by the
 * specified priority.
 *
 * This macro is used to identify the entry called at the specified priority of the
 * application-layer service phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting an application-layer
 * service. The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the application-layer service
 * phase. The value range is [0,5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define APP_SERVICE_INIT_PRI(func, priority) \
    LAYER_INITCALL(func, app_service, "app.service", priority)

/**
 * @brief Identifies the entry for initializing and starting an application-layer feature by the
 * priority 2.
 *
 * This macro is used to identify the entry called at the priority 2 of the application-layer
 * feature phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting an application-layer
 * feature. The type is void (*)(void).
 */
#define APP_FEATURE_INIT(func) LAYER_INITCALL_DEF(func, app_feature, "app.feature")
/**
 * @brief Identifies the entry for initializing and starting an application-layer feature by
 * the specified priority.
 *
 * This macro is used to identify the entry called at the specified priority of the
 * application-layer feature phase of the startup process. \n
 *
 * @param func Indicates the entry function for initializing and starting an application-layer
 * feature. The type is void (*)(void).
 * @param priority Indicates the calling priority when starting the application-layer feature.
 * The value range is [0, 5), and the calling sequence is 0, 1, 2, 3, and 4.
 */
#define APP_FEATURE_INIT_PRI(func, priority) \
    LAYER_INITCALL(func, app_feature, "app.feature", priority)

#define BSP_INIT_PRI(func, priority) LAYER_INITCALL(func, bsp, "bsp", priority);
#define BSP_INIT(func) LAYER_INITCALL_DEF(func, bsp, "bsp")

#define TEST_INIT_PRI(func, priority) LAYER_INITCALL(func, test, "test", priority);
#define TEST_INIT(func) LAYER_INITCALL_DEF(func, test, "test")
#define OHOS_INIT(func) LAYER_INITCALL_DEF(func, ohos, "ohos")
#define RUN_INIT(func) LAYER_INITCALL_DEF(func, run, "run")
#define DYNAMIC_SERVICE_INIT(func) LAYER_INITCALL_DEF(func, dynamic_service, "dynamic.service")
#define SYS_CALL(name, step)                                      \
    do                                                            \
    {                                                             \
        InitCall *initcall = (InitCall *)(SYS_BEGIN(name, step)); \
        InitCall *initend = (InitCall *)(SYS_END(name, step));    \
        for (; initcall < initend; initcall++)                    \
        {                                                         \
            (*initcall)();                                        \
        }                                                         \
    } while (0)

#define MODULE_CALL(name, step)                                      \
    do                                                               \
    {                                                                \
        InitCall *initcall = (InitCall *)(MODULE_BEGIN(name, step)); \
        InitCall *initend = (InitCall *)(MODULE_END(name, step));    \
        for (; initcall < initend; initcall++)                       \
        {                                                            \
            (*initcall)();                                           \
        }                                                            \
    } while (0)

#if (defined(__GNUC__) || defined(__clang__))

#ifdef LAYER_INIT_IN_CONSTRUCTOR

#define SYS_BEGIN(name, step)                                  \
    ({                                                         \
        InitCall *initCall = &init_call_sys_##name##_array[0]; \
        (initCall);                                            \
    })

#define SYS_END(name, step)                                                             \
    ({                                                                                  \
        InitCall *initCall = &init_call_sys_##name##_array[init_call_sys_##name##_end]; \
        (initCall);                                                                     \
    })

#define MODULE_BEGIN(name, step)                           \
    ({                                                     \
        InitCall *initCall = &init_call_##name##_array[0]; \
        (initCall);                                        \
    })
#define MODULE_END(name, step)                                                  \
    ({                                                                          \
        InitCall *initCall = &init_call_##name##_array[init_call_##name##_end]; \
        (initCall);                                                             \
    })

#else

#define SYS_BEGIN(name, step)                                 \
    ({                                                        \
        extern InitCall __zinitcall_sys_##name##_start;       \
        InitCall *initCall = &__zinitcall_sys_##name##_start; \
        (initCall);                                           \
    })

#define SYS_END(name, step)                                 \
    ({                                                      \
        extern InitCall __zinitcall_sys_##name##_end;       \
        InitCall *initCall = &__zinitcall_sys_##name##_end; \
        (initCall);                                         \
    })

#define MODULE_BEGIN(name, step)                          \
    ({                                                    \
        extern InitCall __zinitcall_##name##_start;       \
        InitCall *initCall = &__zinitcall_##name##_start; \
        (initCall);                                       \
    })
#define MODULE_END(name, step)                          \
    ({                                                  \
        extern InitCall __zinitcall_##name##_end;       \
        InitCall *initCall = &__zinitcall_##name##_end; \
        (initCall);                                     \
    })

#endif

#define SYS_INIT(name)     \
    do                     \
    {                      \
        SYS_CALL(name, 0); \
    } while (0)

#define MODULE_INIT(name)     \
    do                        \
    {                         \
        MODULE_CALL(name, 0); \
    } while (0)

#define INIT_APP_CALL(name)         \
    do                              \
    {                               \
        MODULE_CALL(app_##name, 0); \
    } while (0)

#define INIT_TEST_CALL()      \
    do                        \
    {                         \
        MODULE_CALL(test, 0); \
    } while (0)

#elif (defined(__ICCARM__))

#define SYS_NAME(name, step) ".zinitcall.sys." #name #step ".init"
#define MODULE_NAME(name, step) ".zinitcall." #name #step ".init"

#define SYS_BEGIN(name, step) __section_begin(SYS_NAME(name, step))
#define SYS_END(name, step) __section_end(SYS_NAME(name, step))
#define MODULE_BEGIN(name, step) __section_begin(MODULE_NAME(name, step))
#define MODULE_END(name, step) __section_end(MODULE_NAME(name, step))

#pragma section = SYS_NAME(service, 0)
#pragma section = SYS_NAME(service, 1)
#pragma section = SYS_NAME(service, 2)
#pragma section = SYS_NAME(service, 3)
#pragma section = SYS_NAME(service, 4)
#pragma section = SYS_NAME(feature, 0)
#pragma section = SYS_NAME(feature, 1)
#pragma section = SYS_NAME(feature, 2)
#pragma section = SYS_NAME(feature, 3)
#pragma section = SYS_NAME(feature, 4)
#pragma section = MODULE_NAME(bsp, 0)
#pragma section = MODULE_NAME(bsp, 1)
#pragma section = MODULE_NAME(bsp, 2)
#pragma section = MODULE_NAME(bsp, 3)
#pragma section = MODULE_NAME(bsp, 4)
#pragma section = MODULE_NAME(device, 0)
#pragma section = MODULE_NAME(device, 1)
#pragma section = MODULE_NAME(device, 2)
#pragma section = MODULE_NAME(device, 3)
#pragma section = MODULE_NAME(device, 4)
#pragma section = MODULE_NAME(core, 0)
#pragma section = MODULE_NAME(core, 1)
#pragma section = MODULE_NAME(core, 2)
#pragma section = MODULE_NAME(core, 3)
#pragma section = MODULE_NAME(core, 4)
#pragma section = MODULE_NAME(run, 0)
#pragma section = MODULE_NAME(run, 1)
#pragma section = MODULE_NAME(run, 2)
#pragma section = MODULE_NAME(run, 3)
#pragma section = MODULE_NAME(run, 4)

#define SYS_INIT(name)     \
    do                     \
    {                      \
        SYS_CALL(name, 0); \
        SYS_CALL(name, 1); \
        SYS_CALL(name, 2); \
        SYS_CALL(name, 3); \
        SYS_CALL(name, 4); \
    } while (0)

#define MODULE_INIT(name)     \
    do                        \
    {                         \
        MODULE_CALL(name, 0); \
        MODULE_CALL(name, 1); \
        MODULE_CALL(name, 2); \
        MODULE_CALL(name, 3); \
        MODULE_CALL(name, 4); \
    } while (0)

#define INIT_APP_CALL(name) \
    do                      \
    {                       \
        APP_CALL(name, 0);  \
        APP_CALL(name, 1);  \
        APP_CALL(name, 2);  \
        APP_CALL(name, 3);  \
        APP_CALL(name, 4);  \
    } while (0)

#define INIT_TEST_CALL()      \
    do                        \
    {                         \
        MODULE_CALL(test, 0); \
        MODULE_CALL(test, 1); \
        MODULE_CALL(test, 2); \
        MODULE_CALL(test, 3); \
        MODULE_CALL(test, 4); \
    } while (0)

#else
#error Not support current compiler!
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // OHOS_LAYER_INIT_H
/** @} */
