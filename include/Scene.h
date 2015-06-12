#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <random>

#include <glm/glm.hpp>

#include "utils.h"

class Object3D
{
private:
	const Material* material_;
public:
	Object3D(Material* material);
	virtual ~Object3D(){};

	virtual bool intersect(IntersectionData* id, Ray r) const = 0;
	Material material() const;
};

class Sphere : public Object3D
{
private:
	const glm::vec3 POSITION_;
	const float RADIUS_;
public:
	Sphere(glm::vec3 position, float radius, Material* material);
	~Sphere(){};

	bool intersect(IntersectionData* id, Ray r) const;
};

class Plane : public Object3D
{
private:
	const glm::vec3 P0_, P1_, P2_;
public:
	Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, Material* material);
	~Plane(){};

	bool intersect(IntersectionData* id, Ray r) const;
	glm::vec3 getPointOnSurface(float u, float v) const;
};

class LightSource
{
private:
	const Plane emitter_;
public:
	LightSource(
		glm::vec3 p0,
		glm::vec3 p1,
		glm::vec3 p2,
		float emittance,
		SpectralDistribution color);
	~LightSource(){};
	bool intersect(LightSourceIntersectionData* light_id, Ray r);
	glm::vec3 getPointOnSurface(float u, float v);


	const float emittance;
	const SpectralDistribution color;
};

class Scene
{
private:
	std::vector<Object3D*> objects_;
	std::vector<LightSource*> lamps_;

	Material* mirror_;
	Material* glass_;
	Material* air_;

	Material* diffuse_red_;
	Material* diffuse_green_;
	Material* diffuse_blue_;
	Material* diffuse_white_;
	Material* diffuse_gray_;

	std::random_device rd_;
    std::mt19937* gen_;
    std::uniform_real_distribution<float>* dis_;

public:
	Scene();
	~Scene();

	bool intersect(IntersectionData* id, Ray r);
	bool intersectLamp(LightSourceIntersectionData* light_id, Ray r);
	glm::vec3 shake(glm::vec3 r, float power);
	SpectralDistribution traceRay(Ray r, int iteration);
};

#endif // SCENE_H