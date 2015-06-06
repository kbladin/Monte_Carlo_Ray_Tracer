#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include <glm/glm.hpp>

#include "utils.h"

class Object3D
{
public:
	Object3D(Material material);
	virtual ~Object3D(){};

	virtual bool intersect(float* t, Ray r) = 0;

	const Material material_;
};

class Sphere : public Object3D
{
private:
	const glm::vec3 POSITION_;
	const float RADIUS_;
public:
	Sphere(glm::vec3 position, float radius, Material material);
	~Sphere(){};

	bool intersect(float* t, Ray r);
};

class Plane : public Object3D
{
private:
	const glm::vec3 P0_, P1_, P2_;
public:
	Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, Material material);
	~Plane(){};

	bool intersect(float* t, Ray r);
};

class Scene
{
private:
	std::vector<Object3D*> objects_;
public:
	Scene();
	~Scene();

	Object3D* intersect(Ray r);
	SpectralDistribution traceRay(Ray r);
};

#endif // SCENE_H