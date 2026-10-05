#include "sampler.h"

UnitRandom::UnitRandom() : dis(0.0f, 1.0f)
{

}

float UnitRandom::Generate()
{
	return dis(gen);
}

void UnitRandom::seed(uint32_t s)
{
	gen.seed(s);
	dis.reset();
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

Vector3D<float> Sampler::GGXVNDF(Vector3D<float> Ve, float alpha, float r1, float r2)
{
	if (alpha < 1e-3f)
	{
		return Vector3D<float>(0.0f, 0.0f, 1.0f);
	}

	Vector3D<float> Vh(alpha * Ve.x, alpha * Ve.y, Ve.z);
	Vh.normalize();

	float lensq = (Vh.x * Vh.x) + (Vh.y * Vh.y);

	Vector3D<float> T1(1.0f, 0.0f, 0.0f);

	if (Vh.z < 0.99999f)
	{
		T1 = Vector3D<float>(-Vh.y, Vh.x, 0.0f) / std::sqrt(lensq);
	}

	Vector3D<float> T2 = Vh | T1;

	float r = std::sqrt(r1);
	float phi = 2.0f * M_PI * r2;

	float t1 = r * std::cos(phi);
	float t2 = r * std::sin(phi);

	float sqeeze_factor = 0.5f * (1.0f + Vh.z);

	t2 = (1.0f - sqeeze_factor) * std::sqrt(std::max(0.0f, 1.0f - t1 * t1)) + sqeeze_factor * t2;

	float t3 = std::sqrt(std::max(0.0f, 1.0f - (t1 * t1) - (t2 * t2)));

	Vector3D<float> Nh = (T1 * t1) + (T2 * t2) + (Vh * t3);
	Vector3D<float> Ne = Vector3D<float>(alpha * Nh.x, alpha * Nh.y, std::max(1e-6f, Nh.z));
	Ne.normalize();

	return Ne;
}