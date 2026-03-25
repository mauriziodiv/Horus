#pragma once
#include "vec_math.h"
#include <fstream>
#include <string_view>
#include <iostream>

class Shader
{
	public:
		Shader() : color(0.0f, 0.0f, 0.0f) {}
		virtual void setColor(const Vector3D<float> col) { color = col; }
		virtual Vector3D<float> getColor() { return color; }

	private:
		Vector3D<float> color;
};

class Constant : public Shader
{
	public:
		void setColor(const Vector3D<float> col) override { color = col; }
		Vector3D<float> getColor() override { return color; }

	private:
		Vector3D<float> color;
};

class Depth : public Shader
{
	public:

	private:

};