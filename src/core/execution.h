#ifndef THE_CORE_EXECUTION_H
#define THE_CORE_EXECUTION_H

#include <stdbool.h>

struct THE_Config;

extern float deltatime;

void THE_Start(struct THE_Config *config);
void THE_End(void);

#endif

