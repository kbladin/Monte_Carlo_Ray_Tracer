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
	std::random_device rd_;
	std::mt19937* gen_;
	std::uniform_real_distribution<float>* dis_;

	std::vector<Object3D*> objects_;
	std::vector<LightSource*> lamps_;
	std::map<std::string, Material*> materials_;

	KDTree::KDTree<3, KDTreeNode> photon_map_;

	friend struct scene_traverser;
	
  	// Normal path tracing for diffuse ray
	SpectralDistribution traceDiffuseRay(
		Ray r,
		int render_mode,
		IntersectionData id,
		int iteration);
	SpectralDistribution traceLocalDiffuseRay(
		Ray r,
		int render_mode,
		IntersectionData id);
	SpectralDistribution traceIndirectDiffuseRay(
		Ray r,
		int render_mode,
		IntersectionData id,
		int iteration);
	
	// Specular and refractive tracing
	SpectralDistribution traceSpecularRay(
		Ray r,
		int render_mode,
		IntersectionData id,
		int iteration);
	SpectralDistribution traceRefractedRay(
		Ray r,
		int render_mode,
		IntersectionData id,
		int iteration,
		glm::vec3 offset,
		bool inside);

	bool intersect(IntersectionData* id, Ray r);
	bool intersectLamp(LightSourceIntersectionData* light_id, Ray r);
	glm::vec3 shake(glm::vec3 r, float power);
public:
	Scene(const char* file_path);
	~Scene();
	
	enum RenderMode{
	  PHOTON_MAPPING, CAUSTICS, WHITTED_SPECULAR, MONTE_CARLO,
	};
	
	SpectralDistribution traceRay(Ray r, int render_mode, int iteration = 0);
	void buildPhotonMap(const int n_photons);

	int getNumberOfTriangles();
	int getNumberOfObjects();
	int getNumberOfSpheres();
	int getNumberOfPhotons();
};

#endif // SCENE_H