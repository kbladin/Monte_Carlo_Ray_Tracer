#include <string>
#include <iostream>

#include "../include/utils.h"

SpectralDistribution::SpectralDistribution()
{
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		data[i] = 0;
	}
}

float SpectralDistribution::power() const
{
	float sum = 0;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		sum += data[i];
	}
	return sum;
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