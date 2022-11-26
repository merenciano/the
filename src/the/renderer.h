#ifndef THE_RENDERER_H
#define THE_RENDERER_H

#include <string>
#include <vector>

namespace the
{
	struct Material
	{
		Material();

		void SetVertexShader(const std::string& NewShader);
		void SetFragmentShader(const std::string& NewShader);

		std::string vert_shader;
		std::string frag_shader;
		std::vector<float> data;
		std::vector<int> tex;
		int32_t cube_start; //!< Cubemap start index in tex vector.
	};
}

#endif
