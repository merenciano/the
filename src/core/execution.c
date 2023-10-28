#include "execution.h"
#include "core/mem.h"
#include "core/chrono.h"
#include "render/renderer.h"
#include "core/io.h"
#include "scene/camera.h"

#include <stdio.h>

//#define MTR_ENABLED
#include "minitrace.h"

float deltatime;

void THE_Start(struct THE_Config *config)
{
	mtr_init("the_trace.json");
	MTR_META_PROCESS_NAME("The_Profiling");
	MTR_META_THREAD_NAME("Main_Thread");
	MTR_BEGIN("THE", "Init");
	THE_MemInit(config->heap_size);
	THE_IOInit(config->window_title, config->window_width,
		config->window_height, config->vsync);
	THE_InitRender();
	THE_CameraInit(&camera, 70.0f, 300.0f, THE_WindowGetWidth(), THE_WindowGetHeight());
	MTR_END("THE", "Init");

	config->init_func(config->context);

	THE_Chrono frame_chrono = THE_ChronoTime();
	while (!THE_WindowShouldClose()) {
		deltatime = THE_ChronoSeconds(
			THE_ChronoElapsed(frame_chrono));
		frame_chrono = THE_ChronoTime();
		MTR_META_THREAD_NAME("Logic_Thread");
		MTR_BEGIN("THE", "Logic");
		if (!config->update_func(config->context)) {
			break;
		}
		MTR_END("THE", "Logic");

		MTR_BEGIN("THE", "Render");
		THE_RenderFrame();
		MTR_END("THE", "Render");

		MTR_BEGIN("THE", "Swap_Buffers");
		THE_WindowSwapBuffers();
		THE_IOPollEvents();
		THE_RenderEndFrame();
		MTR_END("THE", "Swap_Buffers");
	}
}

void THE_End(void)
{
	mtr_flush();
	mtr_shutdown();
}
