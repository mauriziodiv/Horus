#pragma once
#include "vec_math.h"
#include "ray.h"

class Camera
{

	public:
		Camera();
		Camera(Vector3D<float>);

		void setPosition(Vector3D<float> p);
		Vector3D<float> getPosition();

		void setWindow(float w, float h);
		float getWidth();
		float getHeight();

		Ray genRay(float u, float v);

	private:
		Vector3D<float> position;

		float width;
		float height;

		float aspect_ratio;
		float window_height;
		float window_width;
		float focal_length;

		// ADD SET() TO VECTOR3D
		Vector3D<float> lower_left_corner;
		Vector3D<float> horizontal;
		Vector3D<float> vertical;

};