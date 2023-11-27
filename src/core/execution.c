#include "core/chrono.h"
#include "core/io.h"
#include "core/mem.h"
#include "execution.h"
#include "render/pixels.h"
#include "scene/camera.h"
#include "tools/imgui.h"

// #define MTR_ENABLED
#include "minitrace.h"

float deltatime;

void
nyas_app_start(struct nyas_config *config)
{
	mtr_init("nyas_trace.json");
	MTR_META_PROCESS_NAME("nyas_Profiling");
	MTR_META_THREAD_NAME("Main_Thread");
	MTR_BEGIN("NYAS", "Init");
	nyas_mem_init(config->heap_size);
	nyas_io_init(config->window_title, config->window_width,
	             config->window_height, config->vsync);
	nyas_px_init();
	nyas_camera_init(&camera, 70.0f, 300.0f, nyas_window_width(),
	                 nyas_window_height());
	nyas_imgui_init();
	MTR_END("NYAS", "Init");

	config->init_func(config->context);

	nyas_chrono frame_chrono = nyas_time();
	while (!nyas_window_closed()) {
		deltatime = nyas_time_sec(nyas_elapsed(frame_chrono));
		frame_chrono = nyas_time();
		MTR_META_THREAD_NAME("Logic_Thread");
		MTR_BEGIN("NYAS", "Logic");
		if (!config->update_func(config->context)) {
			break;
		}
		MTR_END("NYAS", "Logic");

		MTR_BEGIN("NYAS", "Render");
		nyas_px_render();
		MTR_END("NYAS", "Render");
		nyas_imgui_draw();

		MTR_BEGIN("NYAS", "Swap_Buffers");
		nyas_window_swap();
		nyas_io_poll();
		nyas_frame_end();
		MTR_END("NYAS", "Swap_Buffers");
	}
}

void
nyas_app_end(void)
{
	mtr_flush();
	mtr_shutdown();
}
