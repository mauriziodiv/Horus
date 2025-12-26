#pragma once
#include <cmath>

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

	void normalize()
	{
		T length = getLength();

		if (length > 0.0001)
		{
			T c = 1.0f / length;

			x = x * c;
			y = y * c;
			z = z * c;
		}
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

	// Dot product
	T operator* (const Vector3D<T>& v) const
	{
		return (x * v.getX()) + (y * v.getY()) + (z * v.getZ());
	}

	// Cross product
	Vector3D operator|(const Vector3D<T>& v) const
	{
		return Vector3D(
			(y * v.getZ()) - (z * v.getY()),
			(z * v.getX()) - (x * v.getZ()),
			(x * v.getY()) - (y * v.getX())
		);
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

template <typename T> class Matrix4X4
{
	public:
		Matrix4X4() {
		
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					i == j ? m4x4[i][j] = T(1) : m4x4[i][j] = T(0);
				}
			}
		}

		void setValue(int x, int y, T value)
		{
			m4x4[x][y] = value;
		}

		T getValue(int x, int y)
		{
			return m4x4[x][y];
		}

		Matrix4X4 operator* (const Matrix4X4& m) const
		{
			Matrix4X4 result;

			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					T sum = T(0);

					for (int n = 0; n < 4; n++)
					{
						sum += m4x4[i][n] * m.getValue(n, j);
					}
					result.setValue(i, j, sum);
				}
			}
			return result;
		}

		Vector3D<T> operator* (const Vector3D<T>& v) const
		{
			T x = (m4x4[0][0] * v.getX()) + (m4x4[0][1] * v.getY()) + (m4x4[0][2] * v.getZ()) + m4x4[0][3];
			T y = (m4x4[1][0] * v.getX()) + (m4x4[1][1] * v.getY()) + (m4x4[1][2] * v.getZ()) + m4x4[1][3];
			T z = (m4x4[2][0] * v.getX()) + (m4x4[2][1] * v.getY()) + (m4x4[2][2] * v.getZ()) + m4x4[2][3];

			return Vector3D<T>(x, y, z);
		}

		static Matrix4X4 rotationX(T angle)
		{
			Matrix4X4 result;

			T c = std::cos(angle);
			T s = std::sin(angle);

			result.setValue(1, 1, c);
			result.setValue(1, 2, -s);
			result.setValue(2, 1, s);
			result.setValue(2, 2, c);

			return result;
		}

		static Matrix4X4 rotationY(T angle)
		{
			Matrix4X4 result;

			T c = std::cos(angle);
			T s = std::sin(angle);

			result.setValue(0, 0, c);
			result.setValue(0, 2, s);
			result.setValue(2, 0, -s);
			result.setValue(2, 2, c);

			return result;
		}

		static Matrix4X4 rotationZ(T angle)
		{
			Matrix4X4 result;

			T c = std::cos(angle);
			T s = std::sin(angle);

			result.setValue(0, 0, c);
			result.setValue(0, 1, -s);
			result.setValue(1, 0, s);
			result.setValue(1, 1, c);

			return result;
		}

	private:
		T m4x4[4][4];
};