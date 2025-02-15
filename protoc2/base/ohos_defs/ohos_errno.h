
#ifndef OHOS_ERRNO_H
#define OHOS_ERRNO_H
#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif /* __cplusplus */
#endif /* __cplusplus */
    /* --------------------------------------------------------------------------------------------*
     * Defintion of error code. The error codes are applicable to both the application and kernel
     *-------------------------------------------------------------------------------------------- */
    enum OHOSLiteErrorCode
    {
        EC_SUCCESS = 0,       /* OK or No error */
        EC_FAILURE = -1,      /* Execution failed */
        EC_BADPTR = -2,       /* Bad pointer value */
        EC_NOMEMORY = -3,     /* No enough memory */
        EC_ADDRESS = -4,      /* Memory or bus address error */
        EC_IOERROR = -5,      /* I/O error */
        EC_BUSBUSY = -6,      /* xxx bus busy */
        EC_NORESPONCE = -7,   /* Function is no responce */
        EC_NODEVICE = -8,     /* No such device */
        EC_INVALID = -9,      /* Invalid argument */
        EC_BUSY = -10,        /* Device or resource busy */
        EC_NOSERVICE = -11,   /* No service */
        EC_PERMISSION = -12,  /* Permission denied */
        EC_NOFILE = -13,      /* No this file */
        EC_NOSPACE = -14,     /* No space left on device */
        EC_NODIR = -15,       /* Not a directory */
        EC_ROFILE = -16,      /* Read-only file system */
        EC_ILLEGALSTATE = -17,  /* ILLEGAL STATE */
        EC_OPBLOCK = -18,     /* Operation would block */
        EC_TIMEOUT = -19,     /* Timer expired */
        EC_COMMU = -20,       /* Communication error on send */
        EC_PROTOCOL = -21,    /* Protocol error */
        EC_CANCELED = -22,    /* Operation Canceled */
        EC_NOKEY = -23,       /* Required key not available */
        EC_NOHANDLER = -24,   /* No handler to execute */
        EC_FLOWCTRL = -25,    /* Flow control */
        EC_NOINIT = -26,      /* device hasn't been initializeed */
        EC_NORESOURCE = -27,  /* resource error */
        EC_LLERRTOP = -127,   /* Low level function error code max value */
        EC_HLERRBASE = -128   /* High level function error code baseline */
    };

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif // OHOS_ERRNO_H
