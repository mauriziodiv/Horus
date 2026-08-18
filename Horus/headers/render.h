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
		Vector3D<float> rayPath(Ray& ray, BVH& bvh, int nBounces, bool includeEmission = true);

		std::vector<LightObject*>& getLights() { return lights; }
		std::vector<AreaLight*>& getAreaLights() { return areaLights; }

	private:

		std::vector<LightObject*> lights;
		std::vector<AreaLight*> areaLights;
		std::vector<MeshLight*> meshLights;

		UnitRandom unitRandom;

		Vector3D<float> toWorld(Vector3D<float> v, Vector3D<float> refVector);
		void reflect(Vector3D<float> hp, Vector3D<float> normal, Vector3D<float> refDir, BVH& bvh, int nBounces, float refGain, Vector3D<float>& col);

		Vector3D<float> subsurfaceWalk(Ray& ray, BVH& bvh, const Medium& medium, float ior, int nBounces);
		bool sampleLightSource(AreaLight*& areaLight, MeshLight*& meshLight, Vector3D<float>& lightPos, Vector3D<float>& lightNormal, Vector3D<float>& lightEmission, float& pdfArea, size_t& lightObjectCount);
};