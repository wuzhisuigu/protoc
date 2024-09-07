#ifndef __SHELL_SERVICE_H__
#define __SHELL_SERVICE_H__

#include "message.h"
#include "ohos_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SHELL_SERVICE "shell"

#include "shell.h"

    typedef enum ShellMessage
    {
        SHELL_MSG_UART_INPUT_READY,
        SHELL_MSG_RBUF_INPUT_READY,
        SHELL_MSG_NTP_NOTIFICATION,
        /** Maximum number of message IDs */
        SHELL_MSG_BUTT
    } ShellMessage;

    typedef struct ShellMsgDef
    {
        uint32 msgId;
        Identity *sId;
        char *insName;
    } ShellMsgDef;

    typedef void (*exit_func)(void *session, int code);

    void ShellRegisterMessage(const ShellCommand *msg);

    void ShellRegisterExit(void *session, exit_func func);

#define SHELL_EXPORT_MSG(msgId_, sId_, desc_, insName_)   \
    SHELL_USED const ShellMsgDef shellMsgDef_##msgId_ = { \
        .msgId = msgId_,                                  \
        .sId = sId_,                                      \
        .insName = insName_,                              \
    };                                                    \
    const char shellCmd_##msgId_[] = #msgId_;             \
    const char shellDesc_##msgId_[] = #desc_;             \
    SHELL_USED const ShellCommand shellMsg_##msgId_ = {   \
        .attr.value = SHELL_CMD_TYPE(SHELL_TYPE_MSG),     \
        .data.msg.name = shellCmd_##msgId_,               \
        .data.msg.value = (void *)&shellMsgDef_##msgId_,  \
        .data.msg.desc = shellDesc_##msgId_};
    // ShellRegisterMessage(&shellMsg##_msgId_)

#define SHELL_REGISTER_MSG(msgId_) ShellRegisterMessage(&shellMsg_##msgId_)

#ifdef __cplusplus
}
#endif

#endif
