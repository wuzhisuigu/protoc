#ifndef __SAMGR_TIMER_H__
#define __SAMGR_TIMER_H__

#include "message.h"
#include "ohos_types.h"
#define SAMGR_TIMER_SERVICE "sagmr_timer"
#define SAMGR_TIMER_REPEAT_INFINITE -1
typedef enum SamgrTimerMessage {
    SAMGR_TIMER_MSG_UPDATE_TIMER,
    SAMGR_TIMER_MSG_TICK,
    /** Maximum number of message IDs */
    SAMGR_TIMER_MSG_BUTT
} SamgrTimerMessage;

typedef struct SamgrTimerParams {
    Identity identity;
    uint32  msgId;
    uint32 interval; 
	int32 repeatCnt;
} SamgrTimerParams;

pHandle SamgrTimer_GetInstance(void);
void SamgrTimer_UpdateTime(pHandle handle,  Identity *targetId, uint32 msgId, uint32 interval, int32 repeatCnt);

#endif
