#include "sampler.h"

UnitRandom::UnitRandom() : dis(0.0f, 1.0f)
{

}

float UnitRandom::Generate()
{
	return dis(gen);
}

Sampler::Sampler()
{

}

// Generates a cosine-weighted random direction in the hemisphere defined by the normal vector.
Vector3D<float> Sampler::cosineWeightSampleHemisphere(float r1, float r2)
{
	float phi = 2.0f * M_PI * r1;

	float x = std::cos(phi) * std::sqrt(r2);
	float y = std::sin(phi) * std::sqrt(r2);
	float z = std::sqrt(1.0f - r2);

	return Vector3D<float>(x, y, z);
}

float Sampler::computePhase(float g, float cosTheta)
{
	float den = 1.0f + (g * g) - (2.0f * g * cosTheta);
	den = std::max(den, 1e-7f);

	return (1.0f - (g * g)) / (4.0f * M_PI * den * std::sqrt(den));
}

Vector3D<float> Sampler::sampleHG(float g, float r1, float r2)
{
	float cosTheta;

	if (std::abs(g) < 1e-3f)
	{
		cosTheta = 1.0f - 2.0f * r1;
	}
	else
	{
		float s = (1.0f - g * g) / (1.0f + g - 2.0f * g * r1);
		cosTheta = (1.0f + g * g - s * s) / (2.0f * g);
	}

	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	float phi = 2.0f * M_PI * r2;

	return Vector3D<float>(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}