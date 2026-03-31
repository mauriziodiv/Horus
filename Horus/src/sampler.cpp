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