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

		Vector3D<float> toWorld(Vector3D<float> v, Vector3D<float> normal);
};