#pragma once
#include "vec_math.h"
#include "ray.h"
#include "hrs.h"
#include "accelerator.h"
#include "sampler.h"

class Integrator
{
	public:
		Integrator(std::vector<LightObject*>& lights) : lights(lights) { };
		Vector3D<float> rayPath(Ray& ray, BVH& bvh, int nBounces);

		std::vector<LightObject*>& getLights() { return lights; }

	private:

		std::vector<LightObject*> lights;
		UnitRandom unitRandom;

		Vector3D<float> toWorld(Vector3D<float> v, Vector3D<float> refVector);
		void reflect(Vector3D<float> hp, Vector3D<float> normal, Vector3D<float> refDir, BVH& bvh, int nBounces, float refGain, Vector3D<float>& col);
};