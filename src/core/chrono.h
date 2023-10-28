#ifndef THE_CORE_CHRONO_H
#define THE_CORE_CHRONO_H

#include <stdint.h>

#define THE_ChronoElapsed(CHRONO) (THE_ChronoTime() - CHRONO)
#define THE_ChronoSeconds(CHRONO) (CHRONO / 1000000000.0f)
#define THE_ChronoMilli(CHRONO) (CHRONO / 1000000)
#define THE_ChronoMicro(CHRONO) (CHRONO / 1000)
#define THE_ChronoNano(CHRONO) (CHRONO)

typedef int64_t THE_Chrono;

THE_Chrono THE_ChronoTime(void);
//THE_Chrono THE_ChronoElapsed(THE_Chrono chrono);

#endif
