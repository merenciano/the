#include "chrono.h"

#include <time.h>

nyas_chrono
nyas_time(void)
{
	struct timespec time;
	clock_gettime(2, &time);
	return time.tv_nsec + time.tv_sec * 1000000000;
}
