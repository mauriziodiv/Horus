#include "BxDF.h"
#include "hrs.h"

void Surface::computeNormal(GeometryObject& ch, Vector3D<float>& normal)
{
	Vector3D<float> normalSample;

	if (ch.getHasTangents() && getNormalSample(normalSample))
	{
		Vector3D<float> T = ch.getTangent();
		Vector3D<float> B = ch.getBitangent();

		T = T - (normal * (T * normal));
		T.normalize();

		Vector3D<float> newB = normal | T;

		if ((newB * B) < 0.0f)
		{
			newB = -newB;
		}

		B = newB;

		normal = (T * normalSample.x) + (B * normalSample.y) + (normal * normalSample.z);
		normal.normalize();
	}

}

bool Surface::setDiffuseColorTex(const std::string& filePath)
{
	return diffuseColorTex.load(filePath);
}

bool Surface::setRoughnessTex(const std::string& filePath)
{
	return roughnessTex.load(filePath);
}

bool Surface::setSubsurfaceGainTex(const std::string& filePath)
{
	return subsurfaceGainTex.load(filePath);
}

bool Surface::setNormalMapTex(const std::string& filePath)
{
	return normalMapTex.load(filePath);
}

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