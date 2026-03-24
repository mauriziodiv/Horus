#pragma once
#include "vec_math.h"
#include "ray.h"
#include "hrs.h"
#include "accelerator.h"

class Integrator
{
	public:
		Integrator() {};
		Vector3D<float> rayPath(Ray& ray, BVH& bvh, int nBounces);

	private:
		
};