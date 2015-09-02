#ifndef PHOTON_MAP
#define PHOTON_MAP

#include "../include/Scene.h"
#include <glm/glm.hpp>
#include "utils.h"

struct Photon
{
	glm::vec3 position;
	SpectralDistribution radiance;
};

class PhotonMap
{
public:
	PhotonMap(Scene* scene);
	~PhotonMap();
	Photon shootPhoton();
private:
	Scene* scene_;
};

#endif