// TODO: Esta clase a la mierda
#ifndef THE_ENTITY_H
#define THE_ENTITY_H

#include "render/renderer.h"

struct THE_Entity {
	float transform[16];
	THE_Mesh mesh;
	THE_Mat mat;
};

typedef struct THE_Entity THE_Entity;

THE_Entity *THE_EntityCreate(void);
THE_Entity *THE_GetEntities(void);
int32_t THE_EntitiesSize(void);
void THE_RenderEntities(THE_Entity *entities, int32_t count);

#endif

