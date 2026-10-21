#pragma once
#include "vec_math.h"
#include "ray.h"
#include "hrs.h"
#include "accelerator.h"
#include "sampler.h"
#include "mesh.h"

class Integrator
{
	public:
		Integrator(std::vector<LightObject*>& lights) : lights(lights) { };
		void addAreaLights(std::vector<AreaLight*>& al);
		void addMeshLights(std::vector<MeshLight*>& ml);
		Vector3D<float> rayPath(Ray& ray, BVH& bvh, int nBounces, bool includeEmission = true, bool isPrimaty = false, float bsdfPdf = 0.0f, Vector3D<float>* diffuseAOV = nullptr, Vector3D<float>* specularAOV = nullptr, Vector3D<float>* emissionAOV = nullptr, Vector3D<float>* refractionAOV = nullptr, Vector3D<float>* subsurfaceScatteringAOV = nullptr, float* alpha = nullptr, Vector3D<float>* normalAOV = nullptr, Vector3D<float>* directLightAOV = nullptr, Vector3D<float>* depthAOV = nullptr, Vector3D<float>* positionAOV = nullptr, Vector3D<float>* indirectAOV = nullptr, Vector3D<float>* lightHitOut = nullptr);

		std::vector<LightObject*>& getLights() { return lights; }
		std::vector<AreaLight*>& getAreaLights() { return areaLights; }

		void seed(uint32_t s) { unitRandom.seed(s); }
		UnitRandom& getUnitRandom() { return unitRandom; }

		Vector3D<float> toLocal(Vector3D<float> vec, Vector3D<float> refVector);

		void setCameraForward(const Vector3D<float>& forward) { cameraForward = forward; }

	private:

		std::vector<LightObject*> lights;
		std::vector<AreaLight*> areaLights;
		std::vector<MeshLight*> meshLights;

		Vector3D<float> cameraForward;

		UnitRandom unitRandom;

		Vector3D<float> toWorld(Vector3D<float> v, Vector3D<float> refVector);
		void reflect(Vector3D<float> hp, Vector3D<float> normal, Vector3D<float> refDir, BVH& bvh, int nBounces, float refGain, Vector3D<float>& col);

		Vector3D<float> subsurfaceWalk(Ray& ray, BVH& bvh, const Medium& medium, float ior, int nBounces);
		bool sampleLightSource(AreaLight*& areaLight, MeshLight*& meshLight, Vector3D<float>& lightPos, Vector3D<float>& lightNormal, Vector3D<float>& lightEmission, float& pdfArea, size_t& lightObjectCount);
		float lightPdfSolidAngle(float pdfArea, float distance, float cosLight);
};