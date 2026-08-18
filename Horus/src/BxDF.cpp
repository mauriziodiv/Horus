#include "BxDF.h"

void Surface::updateChannel(float radius, float color, float& sigmaS, float& sigmaA)
{
	if (radius <= 0.0f)
	{
		sigmaS = 0.0f;
		sigmaA = 0.0f;

		return;
	}

	float sigmaT = 1.0f / radius;
	float albedo = (color < 0.999f) ? color : 0.999f;

	sigmaS = albedo * sigmaT;
	sigmaA = (1.0f - albedo) * sigmaT;
}

void Surface::updateMedium()
{
	Vector3D<float> sigmaS(0.0f, 0.0f, 0.0f);
	Vector3D<float> sigmaA(0.0f, 0.0f, 0.0f);

	updateChannel(subsurfaceRadius.x, subsurfaceColor.x, sigmaS.x, sigmaA.x);
	updateChannel(subsurfaceRadius.y, subsurfaceColor.y, sigmaS.y, sigmaA.y);
	updateChannel(subsurfaceRadius.z, subsurfaceColor.z, sigmaS.z, sigmaA.z);

	medium.setSigmaS(sigmaS);
	medium.setSigmaA(sigmaA);
}