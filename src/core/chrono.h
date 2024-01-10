#ifndef NYAS_CORE_CHRONO_H
#define NYAS_CORE_CHRONO_H

#include <stdint.h>

#define nyas_elapsed(CHRONO) (nyas_time() - (CHRONO))
#define nyas_time_sec(CHRONO) ((CHRONO) / 1000000000.0f)
#define nyas_time_milli(CHRONO) ((CHRONO) / 1000000)
#define nyas_time_micro(CHRONO) ((CHRONO) / 1000)
#define nyas_time_nano(CHRONO) (CHRONO)

typedef int64_t nyas_chrono;
nyas_chrono nyas_time(void);

#endif
