#include "ray.h"

Ray::Ray(Vector3D<float> origin, Vector3D<float> direction) : origin(origin), direction(direction)
{

}

Vector3D<float> Ray::getPointat(float t)
{
	return Vector3D<float>(origin.x + (t * direction.x), origin.y + (t * direction.y), origin.z + (t * direction.z));
}