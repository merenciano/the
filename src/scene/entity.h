// TODO: Esta clase a la mierda
#ifndef NYAS_ENTITY_H
#define NYAS_ENTITY_H

#include "render/pixels.h"

typedef struct nyas_entity {
	float transform[16];
	nyas_mesh mesh;
	nyas_mat mat;
} nyas_entity;

nyas_entity *nyas_entity_create(void);
nyas_entity *nyas_entities(void);
int nyas_entity_count(void);
void nyas_entity_draw(nyas_entity *ntt, int count);

#endif

