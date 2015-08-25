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
	Material* diffuse_cyan_;
	Material* diffuse_white_;
	Material* diffuse_gray_;

	std::random_device rd_;
  std::mt19937* gen_;
  std::uniform_real_distribution<float>* dis_;

  // Bidirectional path tracing for diffuse light
  SpectralDistribution traceBidirectionalDiffuseRay(
  	Ray r,
  	IntersectionData id,
  	int iteration);
  std::vector<std::pair< Ray, IntersectionData > > forwardTraceLightRay(
  	Ray r,
  	int iteration);
  //std::vector<Ray> traceLightRays();

  // Normal path tracing for diffuse ray
	std::vector<std::pair< Ray, IntersectionData > > forwardTraceDiffuseRay(
		Ray r,
		IntersectionData id,
		int iteration);
	std::vector<std::pair< Ray, IntersectionData > > forwardTraceLocalDiffuseRay(
		Ray r,
		IntersectionData id);
	std::vector<std::pair< Ray, IntersectionData > > forwardTraceIndirectDiffuseRay(
		Ray r,
		IntersectionData id,
		int iteration);
	
	// Specular and refractive tracing
	std::vector<std::pair< Ray, IntersectionData > > forwardTraceSpecularRay(
		Ray r,
		IntersectionData id,
		int iteration);
	std::vector<std::pair< Ray, IntersectionData > > forwardTraceRefractedRay(
		Ray r,
		IntersectionData id,
		int iteration,
		glm::vec3 offset,
		bool inside);

  // Normal path tracing for diffuse ray
	SpectralDistribution traceDiffuseRay(
		Ray r,
		IntersectionData id,
		int iteration);
	SpectralDistribution traceLocalDiffuseRay(
		Ray r,
		IntersectionData id);
	SpectralDistribution traceIndirectDiffuseRay(
		Ray r,
		IntersectionData id,
		int iteration);
	
	// Specular and refractive tracing
	SpectralDistribution traceSpecularRay(
		Ray r,
		IntersectionData id,
		int iteration);
	SpectralDistribution traceRefractedRay(
		Ray r,
		IntersectionData id,
		int iteration,
		glm::vec3 offset,
		bool inside);

public:
	Scene();
	~Scene();

	bool intersect(IntersectionData* id, Ray r);
	bool intersectLamp(LightSourceIntersectionData* light_id, Ray r);
	glm::vec3 shake(glm::vec3 r, float power);

	SpectralDistribution traceRay(Ray r, int iteration);
};

#endif // SCENE_H