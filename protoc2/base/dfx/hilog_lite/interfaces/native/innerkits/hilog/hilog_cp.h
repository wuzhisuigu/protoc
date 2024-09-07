#ifndef HIVIEWDFX_HILOG_CPP_H
#define HIVIEWDFX_HILOG_CPP_H

#include "hiview_log.h"

#ifdef __cplusplus

namespace OHOS
{
    namespace HiviewDFX
    {
        struct HiLogLabel
        {
            LogType type;
            unsigned int domain;
            const char *tag;
        };

        class HiLog final
        {
        public:
            static int Debug(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(printf, 2, 3)));
            static int Info(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(printf, 2, 3)));
            static int Warn(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(printf, 2, 3)));
            static int Error(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(printf, 2, 3)));
            static int Fatal(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(printf, 2, 3)));
        };
    } // namespace HiviewDFX
} // namespace OHOS
#endif // __cplusplus

#endif // HIVIEWDFX_HILOG_CPP_H
