#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

struct Ray
{
	glm::vec3 position;
	glm::vec3 direction;
};

struct SpectralDistribution
{
	static const int N_WAVELENGTHS = 3;
	static const int MIN_WAVELENGTH = 300;
	static const int MAX_WAVELENGTH = 700;
	float data[N_WAVELENGTHS];
};

struct Material
{
	glm::vec3 color;
};

#endif // UTILS_H