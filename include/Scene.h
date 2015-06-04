#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

class Scene
{
private:
public:
	Scene();
	~Scene();

	float castRay(glm::vec3 from, glm::vec3 direction);
};

#endif // SCENE_H