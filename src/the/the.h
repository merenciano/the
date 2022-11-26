#ifndef THE_H
#define THE_H

#include <string>

namespace the
{
	struct Mesh
	{
		int _;
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