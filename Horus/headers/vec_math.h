#pragma once
#include <iostream>

template <typename T> class Point
{

	public:

		Point(T v1, T v2) : x(v1), y(v2)
		{

		}

		T getX()
		{
			return x;
		}

		T getY()
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

	Vector3D(T v1, T v2, T v3) : x(v1), y(v2), z(v3) {}

	T getX()
	{
		return x;
	}

	T getY()
	{
		return y;
	}

	T getZ()
	{
		return z;
	}

	T getLength()
	{
		return std::sqrt((x * x) + (y * y) + (z * z));
	}

	Vector3D operator+(const Vector3D& v) const
	{
		return Vector3D(x + v.getX(), y + v.getY(), z + v.getZ());
	}

	Vector3D operator-(const Vector3D& v) const
	{
		return Vector3D(x - v.getX(), y - v.getY(), z - v.getZ());
	}

	Vector3D operator*(T v) const
	{
		return Vector3D(x * v, y * v, z  * v);
	}

	Vector3D operator/(T v) const
	{
		return Vector3D(x / v, y / v, z / v);
	}

private:
	T x;
	T y;
	T z;

};