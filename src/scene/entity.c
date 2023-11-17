#include "entity.h"
#include "mathc.h"
#include "render/rendercommands.h"

#include <stdlib.h>

static nyas_entity entities[256];
static int entities_last;

nyas_entity *
nyas_entity_create()
{
	mat4_identity(entities[entities_last].transform);
	return entities + entities_last++;
}

nyas_entity *
nyas_entities()
{
	return entities;
}

int
nyas_entity_count()
{
	return entities_last;
}

void
nyas_entity_draw(nyas_entity *ntt, int32_t count)
{
	if (!ntt) {
		return;
	}

	nyas_cmd *command_list = nyas_cmd_alloc();
	nyas_cmd *prev = command_list;
	mat4_assign(ntt->mat.ptr, ntt->transform);
	prev->data.draw.material = ntt->mat;
	prev->data.draw.mesh = ntt->mesh;
	prev->execute = nyas_draw_fn;

	for (int i = 1; i < count; ++i) {
		nyas_cmd *com = nyas_cmd_alloc();

		mat4_assign(ntt[i].mat.ptr, ntt[i].transform);
		com->data.draw.material = ntt[i].mat;
		com->data.draw.mesh = ntt[i].mesh;
		com->next = NULL;
		com->execute = nyas_draw_fn;

		prev->next = com;
		prev = com;
	}

	nyas_cmd_add(command_list);
}
