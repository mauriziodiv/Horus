#pragma once
#define _USE_MATH_DEFINES
#include <random>
#include <cmath>
#include "vec_math.h"

class UnitRandom
{
	public:

		UnitRandom();
		float Generate();

	private:

		std::mt19937 gen;
		std::uniform_real_distribution<float> dis;
};

class Sampler
{
	public:
		Sampler();
		static Vector3D<float> cosineWeightSampleHemisphere(float r1, float r2);

	private:
		UnitRandom unitRandom;
};