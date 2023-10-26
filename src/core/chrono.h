#ifndef THE_CORE_CHRONO_H
#define THE_CORE_CHRONO_H

typedef struct THE_Chronometer THE_Chrono;

THE_Chrono *THE_GetChrono(void);
void THE_ChronoStart(THE_Chrono *chrono);
void THE_ChronoEnd(THE_Chrono *chrono);

float THE_ChronoDurationSec(THE_Chrono *chrono);
float THE_ChronoDurationMS(THE_Chrono *chrono);

#endif
