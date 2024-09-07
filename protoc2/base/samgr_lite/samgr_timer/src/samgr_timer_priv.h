
typedef struct SamgrTimerService {
    INHERIT_SERVICE;
    Identity identity;
    // uint32_t status;
    UTILS_DL_LIST timerList;
    uint8 tickCntWr;
    uint8 tickCntRd;
    uint32 lastTick;
} SamgrTimerService;

extern SamgrTimerService samgr_timer_service;