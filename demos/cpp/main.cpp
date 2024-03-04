#include <vector>
#include <cstring>

extern "C" {
#include <nyas.h>
#include <core/utils.h>
}

namespace nyas
{
	using Point = nyas_point;
	using Color = nyas_color;

	template<typename T>
	struct Array
	{
		Array() = delete;
		explicit Array(nyarr* arr) { array((T*)arr->at, arr->count); }
		const std::vector<T>& operator()() const { return array; };
		const T& operator[](int i) { return array[i]; }
	  private:
		std::vector<T> array;
	};

	struct Mesh
	{
		using Handle = nyas_mesh;
		Mesh() : handle(nyas_mesh_create()) {}
		Mesh(const char* path) { handle = nyas_mesh_load_file(path); }
		void Reload(const char* path) { nyas_mesh_reload_file(handle, path); }
		Handle Id() const { return handle; }

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
		Tex::Handle* Textures() { return nyas_shader_tex(handle); }
		Tex::Handle* Cube() { return nyas_shader_cubemap(handle); }
		Handle Id() const { return handle; }

	  private:
		Handle handle;
	};

	struct Framebuffer
	{
		using Handle = nyas_framebuffer;
		Framebuffer() : handle(nyas_fb_create()) {}
		Handle Id() const {return handle; }

		static Framebuffer Default() { Framebuffer fb; fb.handle = 0; return fb; }
	  private:
		Handle handle;
	};

	struct Material
	{
		Shader Shader;
		std::vector<float> Data;
		std::vector<Tex> Texture;
		std::vector<Tex> CubeMap;
		nyas_mat Build() const {
			nyas_mat mat;
			size_t datasz = Data.size() * sizeof(Data[0]);
			size_t texsz = Texture.size() * sizeof(Texture[0]);
			size_t cubesz = CubeMap.size() * sizeof(CubeMap[0]);
			mat.shader = Shader.Id();
			mat.ptr = nyas_falloc(datasz + texsz + cubesz);
			memcpy(mat.ptr, Data.data(), datasz);
			memcpy((char*)mat.ptr + datasz, Texture.data(), texsz);
			memcpy((char*)mat.ptr + datasz + texsz, CubeMap.data(), cubesz);
			return mat;
		}
	};

	struct DrawList
	{
		DrawList() { *this = DrawList(Framebuffer::Default()); }
		DrawList(Framebuffer fb) { dl = nyut_draw_default(); dl.state.target.fb = fb.Id(); }
		void Enable(nyas_draw_flags op) { dl.state.ops.enable |= (1 << op);}
		void Disable(nyas_draw_flags op) { dl.state.ops.disable |= (1 << op);}
		void Blend(nyas_blend_func src, nyas_blend_func dst) { dl.state.ops.blend_src = src; dl.state.ops.blend_dst = dst;}
		void Depth(nyas_depth_func fn) {dl.state.ops.depth_fun = fn;}
		void Cull(nyas_cull_face cull) {dl.state.ops.cull_face = cull;}

		void Add(Mesh mesh, const Material& mat)
		{
			nyas_draw_cmd *cmd = nyarr_nydrawcmd_push(&dl.cmds);
			cmd->material = mat.Build();
			cmd->mesh = mesh.Id();
		}
	  private:
		struct nyas_draw dl;
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
