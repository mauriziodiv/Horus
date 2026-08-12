#pragma once
#include "shader.h"

class BxDF : public Shader
{
	public:

	private:
		
};

struct Medium
{
public:

	Medium() : sigmaS(0.0f, 0.0f, 0.0f), sigmaA(0.0f, 0.0f, 0.0f), g(0.0f) {}

	Vector3D<float> getSigmaT() const
	{
		return sigmaS + sigmaA;
	}

	float getAlbedo() const
	{
		float t = getSigmaT().x;
		float a = (t > 0.0f) ? sigmaS.x / t : 0.0f;
		
		return a;
	}

	void setSigmaS(Vector3D<float> s)
	{
		sigmaS = s;
	}

	void setSigmaA(Vector3D<float> a)
	{
		sigmaA = a;
	}

	void setG(float value)
	{
		g = value;
	}

	Vector3D<float> getSigmaS() const
	{
		return sigmaS;
	}
	
	Vector3D<float> getSigmaA() const
	{
		return sigmaA;
	}

	float getG() const
	{
		return g;
	}

	private:

		Vector3D<float> sigmaS;
		Vector3D<float> sigmaA;

		float g;
};


class Surface : public BxDF
{
	public:
		Surface() : diffuseGain (1.0f), diffuseColor(1.0f, 1.0f, 1.0f), refractionGain(0.0f), roughness(1.0f), IOR(1.0f) {}

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

		void setScattering(Vector3D<float> s)
		{
			medium.setSigmaS(s);
		}

		void setAbsorption(Vector3D<float> a)
		{
			medium.setSigmaA(a);
		}

		void setAnisotropy(float a)
		{
			medium.setG(a);
		}

		Medium getMedium()
		{
			return medium;
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

		Medium medium;
};