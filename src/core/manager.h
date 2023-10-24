#ifndef THE_MANAGER_H
#define THE_MANAGER_H

struct THE_Config;
void THE_InitManager(struct THE_Config *config);
void THE_NextFrame(void);
void THE_StartFrameTimer(void);
float THE_DeltaTime(void);

#endif
