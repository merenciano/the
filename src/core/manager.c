#include "manager.h"

#include "config.h"
#include "chrono.h"
#include "io.h"
#include "mem.h"
#include "render/renderer.h"
#include "render/internalresources.h"
#include "render/rendercommands.h"

static float delta_time;

void THE_InitManager(struct THE_Config *cnfg)
{
	size_t total_mem = THE_MAX_TEXTURES * sizeof(THE_InternalTexture) + THE_MAX_MESHES * sizeof(THE_InternalMesh) + THE_MAX_FRAMEBUFFERS * sizeof(THE_InternalFramebuffer) + THE_MAX_SHADERS * sizeof(THE_InternalShader);
	total_mem += cnfg->render_queue_capacity * 2 * sizeof(void*); // 2 because current and next
	total_mem += cnfg->render_queue_capacity * 2 * sizeof(THE_CommandData);
	total_mem += cnfg->alloc_capacity;
	THE_MemInit(total_mem);
	THE_IOInit(WINDOW_TITLE, cnfg->window_width, cnfg->window_height, cnfg->vsync);
	THE_InitRender();
	delta_time = 0.16f;
}

void THE_NextFrame()
{
	THE_WindowSwapBuffers();
	THE_IOPollEvents();
	THE_SubmitFrame();

	THE_ChronoEnd(THE_GetChrono());
	delta_time = THE_ChronoDurationSec(THE_GetChrono());
	THE_ChronoStart(THE_GetChrono());
}

void THE_StartFrameTimer()
{
	THE_ChronoStart(THE_GetChrono());
}

float THE_DeltaTime()
{
	return delta_time;
}