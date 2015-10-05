#ifndef UTILS_H
#define UTILS_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class SpectralDistribution
{
public:
	SpectralDistribution();
	~SpectralDistribution(){};

	float norm() const;

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
	SpectralDistribution operator*=(const SpectralDistribution& sd);
	SpectralDistribution operator/=(const float& f);
	SpectralDistribution operator*=(const float& f);

	// Currently not used as wavelengths. We only care about three channels,
	// (hence three wavelengths) r, g, b. These would not correspond to real
	// wavelengths at the moments since r is the lowest and blue is the highest.
	static const int N_WAVELENGTHS = 3;
	static const int MIN_WAVELENGTH = 300;
	static const int MAX_WAVELENGTH = 700;
	// The data contains radiance values in all wave lengths.
	float data[N_WAVELENGTHS];
};

struct Material
{
	// The colors are reflectance distribution functions
	// All channels are in the interval [0 , 1]
	SpectralDistribution color_diffuse;
	SpectralDistribution color_specular;
	float reflectance; // [0 , 1]
	float specular_reflectance; // [0 , 1] part of reflectance
	float transmissivity; // [0 , 1]
	float refraction_index; // [1 (air) , 2.4 (diamond)]
	float diffuse_roughness; // [0 , inf) 0 : Lambertian reflector

	static Material air();
};

struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
	Material material; // The material the ray is travelling in
	// The radiance variable is used when tracing rays from light source to photons.
	// When tracing from the camera, the radiance variable will be used for importance. 
	SpectralDistribution radiance; // [Watts / m^2 / steradian]
	bool has_intersected;  // This is used only when forward tracing ray
};

struct Photon
{
	glm::vec3 position;
	glm::vec3 direction_in;
	SpectralDistribution delta_flux; // [Watts]
	static const float RADIUS;
};

struct KDTreeNode
{
	typedef double value_type;
	Photon p;
	size_t index;

	value_type operator[](size_t n) const
	{
		return p.position[n];
	}

	double distance( const KDTreeNode &node)
	{
		return glm::length(node.p.position - p.position);
	}
};

struct IntersectionData
{
	Material material; // Material of the object hit by the ray
	glm::vec3 normal; // Normal of the surface hit by the ray
	float t; // The distance the ray travelled before intersecting
};

struct LightSourceIntersectionData
{
	SpectralDistribution radiosity; // The radiosity of the light source [Watts/m^2]
	float area; // The area of the light source [m^2]
	glm::vec3 normal; // Normal of the surface hit by the ray
	float t; // The distance the ray travelled before intersecting
};

SpectralDistribution evaluatePerfectBRDF(
	SpectralDistribution albedo);
SpectralDistribution evaluateLambertianBRDF(
	glm::vec3 d1,
	glm::vec3 d2,
	glm::vec3 normal,
	SpectralDistribution albedo);
SpectralDistribution evaluateOrenNayarBRDF(
	glm::vec3 d1,
	glm::vec3 d2,
	glm::vec3 normal,
	SpectralDistribution albedo,
	float roughness);


#endif // UTILS_H