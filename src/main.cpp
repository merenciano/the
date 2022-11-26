#include "the.h"
#include "scene.h"

int main()
{
	the::Scene scene;
	auto entt = scene.AddEntity("TestEntity");

	scene.Render();

	return 0;
}
