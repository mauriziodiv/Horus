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
		Surface() : diffuseGain (1.0f), diffuseColor(1.0f, 1.0f, 1.0f), refractionGain(0.0f), roughness(1.0f) {}

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

		void setRefractionGain(float g)
		{
			refractionGain = g;
		}

		void setIOR(float ior)
		{
			IOR = ior;
		}

		float getRoughness()
		{
			return roughness;
		}

		float getRefractionGain()
		{
			return refractionGain;
		}

		float getIOR()
		{
			return IOR;
		}

	private:
		float diffuseGain;
		Vector3D<float> diffuseColor;
		float roughness;
		float refractionGain;
		float IOR;
};