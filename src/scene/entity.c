#include "core/thefinitions.h"
#include "entity.h"
#include "mathc.h"
#include "render/rendercommands.h"

#include <assert.h>
#include <stdlib.h>

static THE_Entity entities[256];
static int32_t entities_last;

THE_Entity *
THE_EntityCreate()
{
	mat4_identity(entities[entities_last].transform);
	return entities + entities_last++;
}

THE_Entity *
THE_GetEntities()
{
	return entities;
}

int32_t
THE_EntitiesSize()
{
	return entities_last;
}

void
THE_RenderEntities(THE_Entity *ntt, int32_t count)
{
	if (!ntt) {
		return;
	}

	THE_RenderCommand *command_list = THE_AllocateCommand();
	THE_RenderCommand *prev = command_list;
	mat4_assign(ntt->mat.ptr, ntt->transform);
	prev->data.draw.material = ntt->mat;
	prev->data.draw.mesh = ntt->mesh;
	prev->execute = THE_DrawExecute;

	for (int i = 1; i < count; ++i) {
		THE_RenderCommand *com = THE_AllocateCommand();

		mat4_assign(ntt[i].mat.ptr, ntt[i].transform);
		com->data.draw.material = ntt[i].mat;
		com->data.draw.mesh = ntt[i].mesh;
		com->next = NULL;
		com->execute = THE_DrawExecute;

		prev->next = com;
		prev = com;
	}

	THE_AddCommands(command_list);
}
