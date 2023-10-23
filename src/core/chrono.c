#include "chrono.h"
#include "thefinitions.h"

#include <complex.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

typedef struct timespec tmspc;

struct THE_Chronometer {
	struct timespec start;
	struct timespec end;
};

/*
	Returns the duration of the crono, if THE_ChronoEnd has been called for the chrono
	it returns end-start. If end has not been called it returns the current duration.
*/
static tmspc Duration(THE_Chrono *chrono)
{
	tmspc ret;
	tmspc tmp;

	if (chrono->end.tv_nsec == 0) {
		clock_gettime(2, &tmp);
	} else {
		tmp = chrono->end;
	}
	ret.tv_sec = tmp.tv_sec - chrono->start.tv_sec;
	ret.tv_nsec = tmp.tv_nsec - chrono->start.tv_nsec;
	return ret;
}

THE_Chrono *THE_GetChrono()
{
	// Ale de momento uno y a mamar.
	static THE_Chrono c = {
		.start = {0},
		.end = {0}
	};
	return &c;
}

void THE_ChronoStart(THE_Chrono *chrono)
{
	//2 is CLOCK_PROCESS_CPUTIME_ID
	clock_gettime(2, &chrono->start);
	chrono->end.tv_nsec = 0;
}

void THE_ChronoEnd(THE_Chrono *chrono)
{
	THE_ASSERT(chrono->end.tv_nsec == 0, "Calling ChronoEnd before ChronoStart");
	clock_gettime(2, &chrono->end);
}

float THE_ChronoDurationSec(THE_Chrono *c)
{
	return THE_ChronoDurationMS(c) / 1000.0f;
}

float THE_ChronoDurationMS(THE_Chrono *c)
{
	tmspc time = Duration(c);
	int64_t nsec = time.tv_nsec + time.tv_sec * 1000000000;
	return (float)nsec / 1000000.0f;
}
