#include "../include/utils.h"

SpectralDistribution::SpectralDistribution()
{

}

SpectralDistribution::~SpectralDistribution()
{
	
}

SpectralDistribution SpectralDistribution::operator+(const SpectralDistribution& sd)
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = data[i] + sd.data[i];
	}
	return to_return;
}

SpectralDistribution SpectralDistribution::operator-(const SpectralDistribution& sd)
{
	SpectralDistribution to_return;
	for (int i = 0; i < N_WAVELENGTHS; ++i)
	{
		to_return.data[i] = data[i] - sd.data[i];
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