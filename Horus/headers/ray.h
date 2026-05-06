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

		void refract(Vector3D<float>& refVector, Vector3D<float> normal, float k, float cos_01,float ratio)
		{
			// Step 4: Compute refracted direction

			//float k = 1.0f - ((ratio * ratio) * (1.0f - (cos_01 * cos_01)));

			refVector.normalize();
			normal.normalize();

			Vector3D<float> t_perp = (refVector + (normal * cos_01)) * ratio; //t⊥ = η ·(d + cos_i · n̂)
			Vector3D<float> t_horizontal = -(normal * std::sqrt(k)); //t_horizontal = -(std::sqrt());//	t‖ = −√k · n̂
			direction = t_perp + t_horizontal; //	t = t⊥ + t
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