#pragma once

template <typename T> class Point
{

	public:

		Point(T v1, T v2) : x(v1), y(v2)
		{

		}

		T GetX()
		{
			return x;
		}

		T GetY()
		{
			return y;
		}


	private:

			T x;
			T y;

};

template <typename T> class Vector3D
{

public:

	Vector3D(T v1, T v2, T v3) : x(v1), y(v2), z(v3)
	{

	}

	T GetX()
	{
		return x;
	}

	T GetY()
	{
		return y;
	}

	T GetZ()
	{
		return z;
	}

private:
	T x;
	T y;
	T z;

};