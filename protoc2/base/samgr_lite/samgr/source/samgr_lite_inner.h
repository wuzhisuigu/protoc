
#ifndef LITE_SAMGR_LITE_INNER_H
#define LITE_SAMGR_LITE_INNER_H

#include "samgr_lite.h"
#include "task_manager.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#ifndef MAX_SERVICE_NUM
#define MAX_SERVICE_NUM 0x7FF0
#endif

#define DEFAULT_SIZE 1
#define SINGLE_SIZE 1
#define MAX_POOL_NUM 8
#define PROPERTY_STEP 8

/* boot strap service use over 2K bytes in ESP32 */
#define DEFAULT_TASK_CFG(pos) {LEVEL_HIGH,  (int16) ((pos) * PROPERTY_STEP + 1), 0x800*2, 25, SHARED_TASK}
#define WDG_SAMGR_INIT_TIME 1000
#define WDG_SVC_REG_TIME 5000
#define WDG_SVC_BOOT_TIME 5000
#define WDG_SVC_TEST_TIME 100000

#define BOOT_FMT(status) ((status) == BOOT_SYS_WAIT ? "Bootstrap core services(count:%d)." : \
((status) == BOOT_APP_WAIT ? "Bootstrap system and application services(count:%d)." : \
((status) == BOOT_DYNAMIC_WAIT ? "Bootstrap dynamic registered services(count:%d)." : \
"Bootstrap other services(count:%d).")))

typedef enum {
    BOOT_SYS = 0,
    BOOT_SYS_WAIT = 1,
    BOOT_APP = 2,
    BOOT_APP_WAIT = 3,
    BOOT_DYNAMIC = 4,
    BOOT_DYNAMIC_WAIT = 5,
} BootStatus;

typedef struct SamgrLiteImpl SamgrLiteImpl;
struct SamgrLiteImpl {
    SamgrLite vtbl;
    MutexId mutex;
    BootStatus status;
    Vector services;
    TaskPool *sharedPool[MAX_POOL_NUM];
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif // LITE_SAMGR_LITE_INNER_H
