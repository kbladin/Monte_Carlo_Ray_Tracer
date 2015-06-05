#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>

#include "utils.h"

class Sphere
{
private:
	const glm::vec3 POSITION_;
	const float RADIUS_;
public:
	Sphere(glm::vec3 position, float radius);
	~Sphere(){};

	bool intersect(glm::vec3* intersection_position, Ray r);
};

class Scene
{
private:
	Sphere* sphere_;
public:
	Scene();
	~Scene();

	SpectralDistribution traceRay(Ray r);
};

#endif // SCENE_H