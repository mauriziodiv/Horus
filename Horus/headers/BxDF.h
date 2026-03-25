#pragma once
#include "shader.h"

class BxDF : public Shader
{
	public:

	private:
		
};


class Surface : public BxDF
{
	public:
		Surface() : diffuseGain (1.0f), diffuseColor(1.0f, 1.0f, 1.0f), roughness(0.0f) {}

	private:
		float diffuseGain;
		Vector3D<float> diffuseColor;
		float roughness;
};