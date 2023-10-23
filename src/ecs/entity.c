#include "entity.h"
#include "core/thefinitions.h"
#include "render/rendercommands.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static THE_Entity entities[256];
static int32_t entities_last;

THE_Entity *THE_EntityCreate()
{
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
	THE_MaterialSetModel(&(entities->mat_data), (float*)&(entities->transform));
	prev->data.draw.shader = entities->mat;
	prev->data.draw.mat = entities->mat_data;
	prev->data.draw.mesh = entities->mesh;
	prev->data.draw.inst_count = 1;
	prev->execute = THE_DrawExecute;
	
	for (int i = 1; i < count; ++i)
	{
		THE_RenderCommand *com = THE_AllocateCommand();

		THE_MaterialSetModel(&(entities[i].mat_data), (float*)&(entities[i].transform));
		com->data.draw.shader = entities[i].mat;
		com->data.draw.mat = entities[i].mat_data;
		com->data.draw.mesh = entities[i].mesh;
		com->data.draw.inst_count = 1;
		com->next = NULL;
		com->execute = THE_DrawExecute;

		prev->next = com;
		prev = com;
	}

	THE_AddCommands(command_list);
}