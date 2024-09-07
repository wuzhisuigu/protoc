
typedef struct EventHubService {
    INHERIT_SERVICE;
    Identity identity;
	// uint32_t status;
	UTILS_DL_LIST senderList;
	UTILS_DL_LIST receiverList;
} EventHubService;

extern  EventHubService event_hub_service;