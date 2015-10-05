#include <string>
#include <iostream>

#include "../include/utils.h"

const float Photon::RADIUS = 0.1;

SpectralDistribution::SpectralDistribution()
{
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		data[i] = 0;
	}
}

float SpectralDistribution::norm() const
{
	float sum = 0;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		sum += data[i];
	}
	return sum / N_WAVELENGTHS;
}

std::ostream& operator<<(std::ostream& os, const SpectralDistribution& sd)
{
	os << "[ ";
	for (int i = 0; i < sd.N_WAVELENGTHS - 1; ++i)
	{
		os << sd.data[i] << ", ";
	}
	os << sd.data[sd.N_WAVELENGTHS - 1] << "]";
	return os;
}

SpectralDistribution operator*(float f, const SpectralDistribution& sd)
{
	return sd * f;
}

float& SpectralDistribution::operator[](const int i)
{
	return data[i];
}

SpectralDistribution SpectralDistribution::operator+(const SpectralDistribution& sd) const
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = data[i] + sd.data[i];
	}
	return to_return;
}

SpectralDistribution SpectralDistribution::operator-(const SpectralDistribution& sd) const
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = data[i] - sd.data[i];
	}
	return to_return;
}

SpectralDistribution SpectralDistribution::operator^(const float& f) const
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = pow(data[i], f);
	}
	return to_return;
}

SpectralDistribution SpectralDistribution::operator/(const float& f) const
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = data[i] / f;
	}
	return to_return;
}

SpectralDistribution SpectralDistribution::operator*(const float& f) const
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = data[i] * f;
	}
	return to_return;
}

SpectralDistribution SpectralDistribution::operator*(const SpectralDistribution& sd) const
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = data[i] * sd.data[i];
	}
	return to_return;
}


SpectralDistribution SpectralDistribution::operator+=(const SpectralDistribution& sd)
{
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		data[i] = data[i] + sd.data[i];
	}
	return *this;
}

SpectralDistribution SpectralDistribution::operator-=(const SpectralDistribution& sd)
{
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		data[i] = data[i] - sd.data[i];
	}
	return *this;
}

SpectralDistribution SpectralDistribution::operator*=(const SpectralDistribution& sd)
{
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		data[i] = data[i] * sd.data[i];
	}
	return *this;
}


SpectralDistribution SpectralDistribution::operator/=(const float& f)
{
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		data[i] = data[i] / f;
	}
	return *this;
}

SpectralDistribution SpectralDistribution::operator*=(const float& f)
{
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		data[i] = data[i] * f;
	}
	return *this;
}

Material Material::air()
{
	Material air;
	air.transmissivity = 1;
	air.refraction_index = 1;
	return air;
}

SpectralDistribution evaluateLambertianBRDF(
	glm::vec3 d1,
	glm::vec3 d2,
	glm::vec3 normal,
	SpectralDistribution albedo)
{
	return albedo / M_PI;
}

SpectralDistribution evaluatePerfectBRDF(
	SpectralDistribution albedo)
{
	return albedo;
}

SpectralDistribution evaluateOrenNayarBRDF(
	glm::vec3 d1,
	glm::vec3 d2,
	glm::vec3 normal,
	SpectralDistribution albedo,
	float roughness)
{
	float sigma2 = roughness * roughness;
	float A = 1 - 0.5 * sigma2 / (sigma2 + 0.57);
	float B = 0.45 * sigma2 / (sigma2 + 0.09);
	float cos_theta_d1 = glm::dot(d1, normal);
	float cos_theta_d2 = glm::dot(d2, normal);
	float theta = glm::acos(cos_theta_d2);
	float theta_d1 = glm::acos(cos_theta_d1);
	float alpha = glm::max(theta, theta_d1);
	float beta = glm::min(theta, theta_d1);
	float cos_d1_d2 = glm::dot(d1, d2);

	return
	albedo / M_PI *
	(A + (B * glm::max(0.0f, cos_d1_d2)) * glm::sin(alpha) * glm::tan(beta));
}