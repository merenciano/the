#include "scene.h"

namespace the
{
	Entity& Scene::AddEntity(std::string_view name)
	{
		return renderables[name];
	}

	void Scene::Render()
	{

	}
}