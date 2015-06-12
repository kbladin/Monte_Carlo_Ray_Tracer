#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <random>

#include <glm/glm.hpp>

#include "utils.h"
#include "Object3D.h"

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