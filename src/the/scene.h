#ifndef THE_SCENE_H
#define THE_SCENE_H

#include "the.h"

#include <string_view>
#include <unordered_map>

namespace the
{
	struct Scene
	{
		/**
		 * @brief Adds a renderable entity to the scene.
		 * The returned reference can be used to modify the entity.
		 * @param name Name of the new entity.
		 * @return const Entity& Reference to the new entity.
		 */
		Entity& AddEntity(std::string_view name);

		/**
		 * @brief Renders all the entities in the scene.
		 */
		void Render();

		std::unordered_map<std::string_view, Entity> renderables;
	};
}

#endif
