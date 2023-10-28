#include "entity.h"
#include "core/thefinitions.h"
#include "mathc.h"
#include "render/rendercommands.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static THE_Entity entities[256];
static int32_t entities_last;

THE_Entity *THE_EntityCreate()
{
	mat4_identity(entities[entities_last].transform);
	return entities + entities_last++;
}

THE_Entity *THE_GetEntities()
{
	return entities;
}

int32_t THE_EntitiesSize()
{
	return entities_last;
}

void THE_RenderEntities(THE_Entity *entities, int32_t count)
{
	THE_ASSERT(entities && count, "Nothing to render.");

	THE_RenderCommand *command_list = THE_AllocateCommand();
	THE_RenderCommand *prev = command_list;
	mat4_assign(entities->mat.ptr, entities->transform);
	prev->data.draw.material = entities->mat;
	prev->data.draw.mesh = entities->mesh;
	prev->execute = THE_DrawExecute;
	
	for (int i = 1; i < count; ++i)
	{
		THE_RenderCommand *com = THE_AllocateCommand();

		mat4_assign(entities[i].mat.ptr, entities[i].transform);
		com->data.draw.material = entities[i].mat;
		com->data.draw.mesh = entities[i].mesh;
		com->next = NULL;
		com->execute = THE_DrawExecute;

		prev->next = com;
		prev = com;
	}

	THE_AddCommands(command_list);
}