#include "execution.h"
#include "core/config.h"
#include "core/manager.h"
#include "core/chrono.h"
#include "render/renderer.h"

#include <stdio.h>

//#define MTR_ENABLED
#include "minitrace.h"

static THE_Callback LogicF;
static THE_Callback CloseF;

void THE_Init(struct THE_Config *cnfg)
{
	mtr_init("the_trace.json");
	MTR_META_PROCESS_NAME("The_Profiling");
	MTR_META_THREAD_NAME("Main_Thread");
	MTR_BEGIN("THE", "Init");
	THE_InitManager(cnfg);
	LogicF = cnfg->logic_func;
	CloseF = cnfg->close_func;

	cnfg->init_func();
	MTR_END("THE", "Init");
}

void THE_Logic()
{
	MTR_META_THREAD_NAME("Logic_Thread");
	MTR_BEGIN("THE", "Logic");
	LogicF();
	MTR_END("THE", "Logic");
}

void THE_Render()
{
	MTR_BEGIN("THE", "Render");
	THE_RenderFrame();
	MTR_END("THE", "Render");
}

void THE_ShowFrame()
{
	MTR_BEGIN("THE", "Swap_Buffers");
	THE_NextFrame();
	MTR_END("THE", "Swap_Buffers");
}

void THE_Close()
{
	CloseF();
	mtr_flush();
	mtr_shutdown();
}
