#ifndef THE_H
#define THE_H

#include <string>

namespace the
{
	struct Mesh
	{
		int _;
	};

	struct Material
	{
		void SetVertexShader(const std::string& NewShader);
		void SetFragmentShader(const std::string& NewShader);
		std::string vert_shader;
		std::string frag_shader;
	};

	struct Transform
	{
		int _;	
	};

	struct Entity 
	{
		Transform tr;
		Mesh mesh;
		Material mat;
	};
}

#endif