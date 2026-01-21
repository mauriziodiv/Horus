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

Vector3D<float> Sampler::cosineWeightsampleHemisphere()
{
	float r1 = unitRandom.Generate();
	float r2 = unitRandom.Generate();
	float phi = 2.0f * M_PI * r1;

	float x = std::cos(phi) * std::sqrt(phi);
	float y = std::sin(phi) * std::sqrt(phi);
	float z = std::sqrt(1.0f - (r2 * r2));

	return Vector3D<float>(x, y, z);
}