#include <ohos_init.h>
#include "hiview_log.h"
#include "ohos_types.h"
#include "service.h"
#include "samgr_lite.h"
#include "timer_adapter.h"
#include "utils_list.h"
#include "shell_service.h"
#include "sm_buffer.h"

#include <memory_adapter.h>
#include <thread_adapter.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "dfx_log.h"

#define DFX_LOG_MODULE_ID OHOS_MODULE_SHELL
// #include "event_hub_service.h"
// #include "ntp_service.h"
#include "shell.h"
#include "shell_ext.h"
#include "bsp_uart.h"

// #define LOGI(...) HILOG_INFO(HILOG_MODULE_APP, __VA_ARGS__)
// #define LOGE(...) HILOG_ERROR(HILOG_MODULE_APP, __VA_ARGS__)

#define SHELL_MSG_CMD_LIST_SIZE 8

#define SHELL_BUFFER_SIZE 128
typedef struct ShellService {
    INHERIT_SERVICE;
    Identity identity;
    Shell shell;
    char shell_buf[SHELL_BUFFER_SIZE];
    // uint32_t status;
} ShellService;

const ShellCommand *msg_cmd_list[SHELL_MSG_CMD_LIST_SIZE];
int msg_cmd_count;

void shellSendMsg(Shell *shell, const ShellCommand *command, int argc, char **argv)
{
    Request request = { 0 };
    int32 a0 = 0, a1 = 0;
    ShellMsgDef *msg = command->data.msg.value;

    if (argc > 1) {
        a0 = shellExtParseNumber(argv[1]);
    }
    if (argc > 2) {
        a1 = shellExtParseNumber(argv[2]);
    }

    (void)shell;
    // ShellService *service = (ShellService*)((uint8*)shell - OFFSET_OF_FIELD(ShellService, shell));
    // MSG_NAME num1 num2 [insName]
    request.msgId = msg->msgId;
    request.msgValue = a0;
    request.data = (void *)(intptr)a1;
    SAMGR_SendRequest(msg->sId, &request, NULL);
}

void ShellRegisterMessage(const ShellCommand *msg)
{
    int i = 0;
    for (; i < SHELL_MSG_CMD_LIST_SIZE; i++) {
        if (!msg_cmd_list[i]) {
            msg_cmd_list[i] = msg;
            msg_cmd_count++;
            break;
        }
    }
}

void onUartInput(void *context)
{
    ShellService *service = (ShellService *)context;
    Request request = { 0 };
    request.msgId = SHELL_MSG_UART_INPUT_READY;
    SAMGR_SendRequest(&service->identity, &request, NULL);
}

unsigned short shell_write(char *msg, unsigned short len)
{
    bsp_shell_output(msg, len);
    (void)msg;
    return len;
}

void shell_ps(int id)
{
    uint32 i;
    int ret;
    struct ThreadInfo info;
    LOGI("id    used/size      name");
    for (i = 0; i < THREAD_NUM_MAX; i++) {
        ret = THREAD_GetThreadInfo(i, &info);
        if (ret == 0) {
            LOGI("%02d  %06d/%06d  %s", i, info.stackWM, info.stackSize, info.name);
        }
    }
    (void)id;
}

static void initUart(ShellService *service)
{
    service->shell.write = shell_write;
    service->shell.ps = shell_ps;
    shellInit(&service->shell, service->shell_buf, SHELL_BUFFER_SIZE);
    bsp_uart_set_rx_cb(BSP_UART_ID_CONSOLE, onUartInput, service);
}

static const char *GetName(Service *service);
static BOOL Initialize(Service *service, Identity identity);
static TaskConfig GetTaskConfig(Service *service);
static BOOL MessageHandle(Service *service, Request *msg);
ShellService shell_service = {
    .GetName = GetName,
    .Initialize = Initialize,
    .MessageHandle = MessageHandle,
    .GetTaskConfig = GetTaskConfig,
};

void ShellRegisterExit(void *session, exit_func func)
{
    shell_service.shell.session = session;
    shell_service.shell.exit = func;
}

static const char *GetName(Service *service)
{
    (void)service;
    return SHELL_SERVICE;
}
static BOOL Initialize(Service *service_, Identity identity)
{
    ShellService *service = (ShellService *)service_;

    service->identity = identity;

    /*   
    Identity ntpId;
    SAMGR_GetInstance()->GetServiceId(NTP_SERVICE, &ntpId);
    pHandle eventHub = EventHub_GetInstance();
    EventHub_ListenToEvent(eventHub, &ntpId, &service->identity, SHELL_MSG_NTP_NOTIFICATION);
 */
    initUart(service);
    return TRUE;
}

static void onRbufInputReady(ShellService *service, void *rbuf)
{
    uint32 i = 0, n;
    char *c;
    do {
        c = (char *)sm_buffer_read_peak(rbuf, &n);
        if (n == 0) {
            break;
        }
        do {
            shellHandler(&service->shell, c[0]);
            c++;
            i++;
        } while (i < n);
        sm_buffer_read_commit(rbuf, n);
    } while (1);
}

static void onUartInputReady(ShellService *service)
{
    int n;
    char c;
    do {
        n = bsp_uart_read(BSP_UART_ID_CONSOLE, &c, 1);
        if (n == 0) {
            break;
        }
        shellHandler(&service->shell, c);
    } while (n > 0);
}

static BOOL MessageHandle(Service *service_, Request *msg)
{
    // LOGI("[Shell] msgId<%d>\n",  msg->msgId);
    ShellService *service = (ShellService *)service_;
    switch (msg->msgId) {
    case SHELL_MSG_UART_INPUT_READY:
        // shellHandler(&service->shell, msg->msgValue);
        onUartInputReady(service);
        break;
    case SHELL_MSG_RBUF_INPUT_READY:
        // shellHandler(&service->shell, msg->msgValue);
        onRbufInputReady(service, msg->data);
        break;

    case SHELL_MSG_NTP_NOTIFICATION:
        // onNtpReady(service);
        break;
    default:
        break;
    }

    return FALSE;
}

static TaskConfig GetTaskConfig(Service *service)
{
    (void)service;
    // SHARED_TASK use config in DEFAULT_TASK_CFG, don't use the setting here
    TaskConfig config = { LEVEL_HIGH, PRI_BELOW_NORMAL, 0, 0, SHARED_TASK };
    return config;
}

/* module entry function can not be static, it is forced to be linked into the target executable
  by linker option -u */
void DfxShellInit(void)
{
    ShellService *service = &shell_service;
    SAMGR_GetInstance()->RegisterService((Service *)service);
}
TEST_INIT(DfxShellInit);
