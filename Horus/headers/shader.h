#pragma once
#include "vec_math.h"
#include <fstream>
#include <string_view>
#include <iostream>

class Shader
{
	public:
		Shader() : color(0.0f, 0.0f, 0.0f) {}

		void setColor(const Vector3D<float> col) { color = col; }
		Vector3D<float> getColor() const { return color; }

	private:
		Vector3D<float> color;
};

class Constant : public Shader
{
	public:

	private:
};