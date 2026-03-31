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
		Surface() : diffuseGain (1.0f), diffuseColor(1.0f, 1.0f, 1.0f), roughness(1.0f) {}

		void setDiffuseGain(float g)
		{
			diffuseGain = g;
		}

		float getDiffuseGain()
		{
			return diffuseGain;
		}

		void setDiffuseColor(Vector3D<float> col)
		{
			diffuseColor = col;
		}

		Vector3D<float> getDiffuseColor()
		{
			return diffuseColor;
		}

		void setRoughness(float r)
		{
			roughness = r;
		}

		float getRoughness()
		{
			return roughness;
		}

	private:
		float diffuseGain;
		Vector3D<float> diffuseColor;
		float roughness;
};