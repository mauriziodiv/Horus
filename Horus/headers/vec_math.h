#pragma once
#include <cmath>
#include <cstdint>

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

			T x;
			T y;

};

template <typename T> class Vector3D
{

public:

	Vector3D() : x(T(0)), y(T(0)), z(T(0)) {}
	Vector3D(T v1, T v2, T v3) : x(v1), y(v2), z(v3) {}

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
		return Vector3D(x + v.x, y + v.y, z + v.z);
	}

	Vector3D& operator+=(const Vector3D& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3D operator-(const Vector3D& v) const
	{
		return Vector3D(x - v.x, y - v.y, z - v.z);
	}

	Vector3D& operator-=(const Vector3D& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3D operator*(T v) const
	{
		return Vector3D(x * v, y * v, z  * v);
	}

	Vector3D& operator*=(T v)
	{
		x *= v;
		y *= v;
		z *= v;
		return *this;
	}

	// Dot product
	T operator*(const Vector3D<T>& v) const
	{
		return (x * v.x) + (y * v.y) + (z * v.z);
	}

	// Cross product
	Vector3D operator|(const Vector3D<T>& v) const
	{
		return Vector3D(
			(y * v.z) - (z * v.y),
			(z * v.x) - (x * v.z),
			(x * v.y) - (y * v.x)
		);
	}

	Vector3D operator/(T v) const
	{
		return Vector3D(x / v, y / v, z / v);
	}

	Vector3D& operator/=(T v)
	{
		x /= v;
		y /= v;
		z /= v;
		return *this;
	}

	T x;
	T y;
	T z;

};

template <typename T> class Matrix4X4
{
	public:
		Matrix4X4() {
		
			for (int32_t i = 0; i < 4; i++)
			{
				for (int32_t j = 0; j < 4; j++)
				{
					i == j ? m4x4[i][j] = T(1) : m4x4[i][j] = T(0);
				}
			}
		}

		void setValue(int32_t x, int32_t y, T value)
		{
			m4x4[x][y] = value;
		}

		T getValue(int32_t x, int32_t y)
		{
			return m4x4[x][y];
		}

		Matrix4X4 operator* (const Matrix4X4& m) const
		{
			Matrix4X4 result;

			for (int32_t i = 0; i < 4; i++)
			{
				for (int32_t j = 0; j < 4; j++)
				{
					T sum = T(0);

					for (int32_t n = 0; n < 4; n++)
					{
						sum += m4x4[i][n] * m.getValue(n, j);
					}
					result.setValue(i, j, sum);
				}
			}
			return result;
		}

		Matrix4X4& operator*= (const Matrix4X4& m)
		{
			*this = *this * m;
			return *this;
		}

		Vector3D<T> operator* (const Vector3D<T>& v) const
		{
			T x = (m4x4[0][0] * v.x) + (m4x4[0][1] * v.y) + (m4x4[0][2] * v.z) + m4x4[0][3];
			T y = (m4x4[1][0] * v.x) + (m4x4[1][1] * v.y) + (m4x4[1][2] * v.z) + m4x4[1][3];
			T z = (m4x4[2][0] * v.x) + (m4x4[2][1] * v.y) + (m4x4[2][2] * v.z) + m4x4[2][3];

			return Vector3D<T>(x, y, z);
		}

		static Matrix4X4 Translation(T tx, T ty, T tz)
		{
			Matrix4X4 result;
			result.setValue(0, 3, tx);
			result.setValue(1, 3, ty);
			result.setValue(2, 3, tz);
			return result;
		}

		static Matrix4X4 RotationX(T angle)
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

		static Matrix4X4 RotationY(T angle)
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

		static Matrix4X4 RotationZ(T angle)
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

		static Matrix4X4 Scaling(T sx, T sy, T sz)
		{
			Matrix4X4 result;
			result.setValue(0, 0, sx);
			result.setValue(1, 1, sy);
			result.setValue(2, 2, sz);
			return result;
		}

		T minor(int32_t row, int32_t col) const
		{
			T m[3][3];
			int32_t mRow = 0, mCol = 0;
			for (int32_t i = 0; i < 4; i++)
			{
				if (i == row) continue;
				mCol = 0;
				for (int32_t j = 0; j < 4; j++)
				{
					if (j == col) continue;
					m[mRow][mCol] = m4x4[i][j];
					mCol++;
				}
				mRow++;
			}
			return 
				m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
				m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
				m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
		}

		T determinant() const
		{
			return
				m4x4[0][0] * minor(0, 0) -
				m4x4[0][1] * minor(0, 1) +
				m4x4[0][2] * minor(0, 2) -
				m4x4[0][3] * minor(0, 3);
		}

		Matrix4X4 inverse(bool& valid) const
		{
			Matrix4X4 result;
			T det = determinant();

			valid = true;

			if (std::abs(det) < 1e-6)
			{
				valid = false;
				return result;
			}

			for (int32_t i = 0; i < 4; i++)
			{
				for (int32_t j = 0; j < 4; j++)
				{
					T cofactor = ((i + j) % 2 == 0 ? 1 : -1) * minor(i, j);
					result.setValue(j, i, cofactor / det); // Transpose while setting
				}
			}

			return result;
		}

		static Matrix4X4 LookAt(const Vector3D<T>& eye, const Vector3D<T>& center, const Vector3D<T>& up)
		{
			Vector3D<T> f = center - eye;
			f.normalize();

			Vector3D<T> s = f | up;
			s.normalize();

			Vector3D<T> u = s | f;
			Matrix4X4 result;

			result.setValue(0, 0, s.x);
			result.setValue(0, 1, s.y);
			result.setValue(0, 2, s.z);
			result.setValue(1, 0, u.x);
			result.setValue(1, 1, u.y);
			result.setValue(1, 2, u.z);
			result.setValue(2, 0, -f.x);
			result.setValue(2, 1, -f.y);
			result.setValue(2, 2, -f.z);
			result.setValue(0, 3, - (s * eye));
			result.setValue(1, 3, - (u * eye));
			result.setValue(2, 3, f * eye);
			return result;
		}

		static Matrix4X4 Perspective(T fovY, T aspect, T near, T far)
		{
			Matrix4X4 result;
			T f = T(1) / std::tan(fovY / T(2));
			result.setValue(0, 0, f / aspect);
			result.setValue(1, 1, f);
			result.setValue(2, 2, (far + near) / (near - far));
			result.setValue(2, 3, (T(2) * far * near) / (near - far));
			result.setValue(3, 2, -T(1));
			result.setValue(3, 3, T(0));
			return result;
		}

		Vector3D<T> screenToNDC(const Vector3D<T>& screenPos, int32_t screenWidth, int32_t screenHeight) const
		{
			T x = (screenPos.x / T(screenWidth)) * T(2) - T(1);
			T y = T(1) - (screenPos.y / T(screenHeight)) * T(2); // Invert Y from screen space
			return Vector3D<T>(x, y, screenPos.z);
		}

		Vector3D<T> NDCToScreen(const Vector3D<T>& ndc, int32_t screenWidth, int32_t screenHeight) const
		{
			T x = ((ndc.x + T(1)) / T(2)) * T(screenWidth);
			T y = ((T(1) - ndc.y) / T(2)) * T(screenHeight); // Invert Y for screen space
			return Vector3D<T>(x, y, ndc.z);
		}

		static Vector3D<T> Unproject(const Vector3D<T>& screenPos, const Matrix4X4& viewMatrix, const Matrix4X4& projectionMatrix, int32_t screenWidth, int32_t screenHeight)
		{
			T ndcX = (screenPos.x / T(screenWidth)) * T(2) - T(1);
			T ndcY = T(1) - (screenPos.y / T(screenHeight)) * T(2);
			T ndcZ = screenPos.z * T(2) - T(1);  // Convert [0,1] to [-1,1]

			bool valid;
			Matrix4X4<T> invVP = (projectionMatrix * viewMatrix).inverse(valid);

			if (!valid)
			{
				return Vector3D<T>();
			}

			T x = ndcX, y = ndcY, z = ndcZ, w = T(1);

			T worldX = invVP.getValue(0, 0) * x + invVP.getValue(0, 1) * y + invVP.getValue(0, 2) * z + invVP.getValue(0, 3) * w;
			T worldY = invVP.getValue(1, 0) * x + invVP.getValue(1, 1) * y + invVP.getValue(1, 2) * z + invVP.getValue(1, 3) * w;
			T worldZ = invVP.getValue(2, 0) * x + invVP.getValue(2, 1) * y + invVP.getValue(2, 2) * z + invVP.getValue(2, 3) * w;
			T worldW = invVP.getValue(3, 0) * x + invVP.getValue(3, 1) * y + invVP.getValue(3, 2) * z + invVP.getValue(3, 3) * w;

			worldX /= worldW;
			worldY /= worldW;
			worldZ /= worldW;

			return Vector3D<T>(worldX, worldY, worldZ);
		}

	private:
		T m4x4[4][4];
};