
#ifndef OHOS_TYPES_H
#define OHOS_TYPES_H

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

    /*
     * Defintion of basic data types.
     * The data types are applicable to both the application and kernel.
     */
    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;
    typedef signed char int8;
    typedef short int16;
    typedef int int32;

#ifndef _M_IX86
    typedef unsigned long long uint64;
    typedef long long int64;
#else
typedef unsigned __int64 uint64;
typedef __int64 int64;
#endif

#if __LP64__
    /* Linux and Mac OSX x86_64: LONG,POINTER are 64-bit */
    typedef long int intptr;
    typedef unsigned long int uintptr;
#elif __WIN64
typedef long long int intptr;
typedef unsigned long long int uintptr;
#else

typedef long int intptr;
typedef unsigned long int uintptr;
#endif

    typedef int boolean;

    typedef void *pHandle;

    typedef unsigned int BOOL;

#ifndef TRUE
#define TRUE 1L
#endif

#ifndef FALSE
#define FALSE 0L
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void *)0)
#endif
#endif

#define OHOS_SUCCESS 0
#define OHOS_FAILURE (-1)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* OHOS_TYPES_H */
