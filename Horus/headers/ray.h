#pragma once
#include "vec_math.h"

class Ray
{
	public:
		Ray() : origin(0.0f, 0.0f, 0.0f), direction(0.0f, 0.0f, 1.0f) {}
		Ray(Vector3D<float> origin, Vector3D<float> direction);
		Vector3D<float> getPointat(float t);

		void setOrigin(const Vector3D<float>& o)
		{
			origin = o;
		}

		Vector3D<float> getOrigin() const
		{
			return origin;
		}

		void setDirection(const Vector3D<float>& d)
		{
			direction = d;
		}

		Vector3D<float> getDirection() const
		{
			return direction;
		}

		void reflect(Vector3D<float>& refVector)
		{
			refVector.normalize();
				
			direction = direction - ((refVector * (direction * refVector)) * 2.0f);
		}

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

		Vector3D<float> origin;
		Vector3D<float> direction;

		float tMin = 0.001f;
		float tMax = 10000.0f;
};