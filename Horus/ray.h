#pragma once
#include "vec_math.h"

class Ray
{
	public:
		Ray(Vector3D<float>, Vector3D<float>);
		Vector3D<float> getPointat(float t);

		void setTMin(float tmin)
		{
			tMin = tmin;
		}

		float getTMin()
		{
			return tMin;
		}

		void setTMax(float tmax)
		{
			tMax = tmax;
		}

		float getTMax()
		{
			return tMax;
		}

	private:
		Vector3D<float> origin;
		Vector3D<float> direction;

		float tMin = 0.001f;
		float tMax = 10000.0f;
};