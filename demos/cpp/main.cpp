extern "C" {
#include <nyas.h>
}

namespace nyas
{
	using Point = nyas_point;

	struct Mesh
	{
		using Handle = nyas_mesh;
		Mesh() : handle(nyas_mesh_create()) {}
		Mesh(const char* path) { handle = nyas_mesh_load_file(path); }
		void Reload(const char* path) { nyas_mesh_reload_file(handle, path); }

	  private:
		Handle handle;
	};

	struct Tex
	{
		using Handle = nyas_tex;
		using Descriptor = nyas_texture_desc;
		Tex() : handle(nyas_tex_create()) {}
		Tex(Handle h) : handle(h) {}
		Tex(Descriptor* desc) {handle = nyas_tex_create(); nyas_tex_set(handle, desc);}
		Tex(Descriptor* desc, const char *path) {handle = nyas_tex_create(); nyas_tex_load(handle, desc, path);};
		Point Size() {return nyas_tex_size(handle);}

	  private:
		Handle handle;
	};

	struct Shader
	{
		using Handle = nyas_shader;
		using Descriptor = nyas_shader_desc;
		Shader(Descriptor* desc) : handle(nyas_shader_create(desc)) {}
		void Reload() { nyas_shader_reload(handle); }
		void *Data() { return nyas_shader_data(handle); }
		Tex::Handle* Tex() { return nyas_shader_tex(handle); }
		Tex::Handle* Cube() { return nyas_shader_cubemap(handle); }

	  private:
		Handle handle;
	};

	struct Framebuffer
	{
		using Handle = nyas_framebuffer;
	};
}

int main()
{
	nyas_io_init("Nyaspla", {800, 600});

	while (!nyas_io->window_closed)
	{
		nyas_io_poll();
	}

	return 0;
}
