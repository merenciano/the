// TODO: Esta clase a la mierda
#ifndef THE_ENTITY_H
#define THE_ENTITY_H

#include "render/renderer.h"

typedef struct THE_Entity {
	float transform[16];
	THE_Mesh mesh;
	THE_Material mat;
} THE_Entity;

THE_Entity *THE_EntityCreate(void);
THE_Entity *THE_GetEntities(void);
int THE_EntitiesSize(void);
void THE_RenderEntities(THE_Entity *ntt, int count);

#endif

