#include "camera.h"

Camera::Camera() : position(0.0f, 0.0f, -1.0f), width(200.0f), height(100.0f), aspect_ratio(2.0f), window_height(2.0f), window_width(4.0f), focal_length(1.0f), lower_left_corner(-2.0f, -1.0f, -2.0f), horizontal(4.0f, 0.0f, 0.0f), vertical(0.0f, 2.0f, 0.0f)
{

}

Camera::Camera(Vector3D<float> p)
{
	position = p;
}

void Camera::setPosition(Vector3D<float> p)
{
	position = p;
}

Vector3D<float> Camera::getPosition()
{
	return position;
}

// Sets the window dimensions and calculates the aspect ratio, window size, and camera parameters accordingly.
void Camera::setWindow(float w, float h)
{
	width = w;
	height = h;

	aspect_ratio = width / height;
	window_height = 2.0f;
	window_width = window_height * aspect_ratio;

	lower_left_corner = Vector3D<float>(position.x - (window_width * 0.5), position.y - (window_height * 0.5), position.z - focal_length);
	horizontal = Vector3D<float>(window_width, 0.0f, 0.0f);
	vertical = Vector3D<float>(0.0f, window_height, 0.0f);
}

float Camera::getWidth()
{
	return width;
}

float Camera::getHeight()
{
	return height;
}

// Generates a ray from the camera's position through the point on the image plane defined by the normalized coordinates (u, v).
Ray Camera::genRay(float u, float v)
{
	 Vector3D<float> direction = lower_left_corner + (horizontal * u) + (vertical * v) - position;
	 direction.normalize();
	
	 return Ray(position, direction);
}