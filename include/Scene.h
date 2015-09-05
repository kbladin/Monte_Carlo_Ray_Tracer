#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <map>
#include <random>

#include <glm/glm.hpp>

#include "utils.h"
#include "Object3D.h"
#include "../external_libraries/common_include/kdtree++/kdtree.hpp"

class Scene
{
private:
	int render_mode_;

	std::random_device rd_;
	std::mt19937* gen_;
	std::uniform_real_distribution<float>* dis_;

	std::vector<Object3D*> objects_;
	std::vector<LightSource*> lamps_;
	std::map<std::string, Material*> materials_;

	KDTree::KDTree<3, KDTreeNode> photon_map_;

	friend struct scene_traverser;
	
	/*
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
	*/
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

	// Specular and refractive tracing
	Photon traceSpecularPhoton(
		Ray r,
		IntersectionData id,
		int iteration);
	Photon traceRefractedPhoton(
		Ray r,
		IntersectionData id,
		int iteration,
		glm::vec3 offset,
		bool inside);
	Photon tracePhoton(Ray r, int iteration);

	bool intersect(IntersectionData* id, Ray r);
	bool intersectLamp(LightSourceIntersectionData* light_id, Ray r);
	glm::vec3 shake(glm::vec3 r, float power);
public:
	Scene(const char* file_path);
	~Scene();
	
	enum RenderMode{
	  PHOTON_MAPPING, CAUSTICS, WHITTED_SPECULAR, MONTE_CARLO,
	};
	
	void setRenderMode(int render_mode);

	SpectralDistribution traceRay(Ray r, int iteration = 0);
	void buildPhotonMap(const int n_photons);
};

#endif // SCENE_H