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
		Vector3D<float> cosineWeightsampleHemisphere();

	private:
		UnitRandom unitRandom;
};