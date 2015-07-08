#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class SpectralDistribution
{
public:
	SpectralDistribution();
	~SpectralDistribution(){};

	// Various operators for SpectralDistribution
	friend std::ostream& operator<<(
		std::ostream& os,
		const SpectralDistribution& sd);
	friend SpectralDistribution operator*(
		float f,
		const SpectralDistribution& sd);
	float& operator[](const int i);
	SpectralDistribution operator+(const SpectralDistribution& sd) const;
	SpectralDistribution operator-(const SpectralDistribution& sd) const;
	SpectralDistribution operator^(const float& f) const;
	SpectralDistribution operator/(const float& f) const;
	SpectralDistribution operator*(const float& f) const;
	SpectralDistribution operator*(const SpectralDistribution& sd) const;
	SpectralDistribution operator+=(const SpectralDistribution& sd);
	SpectralDistribution operator-=(const SpectralDistribution& sd);
	SpectralDistribution operator/=(const float& f);

	// Currently not used as wavelengths. We only care about three channels,
	// (hence three wavelengths) r, g, b. These would not correspond to real
	// wavelengths at the moments since r is the lowest and blue is the highest.
	static const int N_WAVELENGTHS = 3;
	static const int MIN_WAVELENGTH = 300;
	static const int MAX_WAVELENGTH = 700;
	float data[N_WAVELENGTHS];
};

struct Material
{
	SpectralDistribution color_diffuse;
	SpectralDistribution color_specular;
	float reflectance; // [0 , 1]
	float specular_reflectance; // [0 , 1] part of reflectance
	float transmissivity; // [0 , 1]
	float refraction_index; // [1 (air) , 2.4 (diamond)]
	float polish_power; // [1 , inf) High => near perfect reflection
	float clearness_power; // [1 , inf) High => near perfect refraction

	static Material air();
};

struct Ray
{
	glm::vec3 position;
	glm::vec3 direction;
	Material material; // The material the ray is travelling in
};

struct IntersectionData
{
	Material material; // Material of the object hit by the ray
	glm::vec3 normal; // Normal of the surface hit by the ray
	float t; // The distance the ray travelled before intersecting
};

struct LightSourceIntersectionData
{
	SpectralDistribution color; // The color of the light source
	float emittance; // The emittance of the light source
	glm::vec3 normal; // Normal of the surface hit by the ray
	float t; // The distance the ray travelled before intersecting
};

#endif // UTILS_H