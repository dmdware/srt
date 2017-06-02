#ifndef VEC3D_H
#define VEC3D_H

#include "../phys/physics.h"

class Matrix;

class Vec3d
{
public:
	double x, y, z;

	Vec3d()
	{
		x = y = z = 0;
	}

	Vec3d(double X, double Y, double Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	Vec3d(const double* values)
	{
		set(values);
	}

	bool operator==(const Vec3d vVector) const
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z)
			return true;

		return false;
	}

	bool operator!=(const Vec3d vVector) const
	{
		if(x == vVector.x && y == vVector.y && z == vVector.z)
			return false;

		return true;
	}

	Vec3d operator+(const Vec3d vVector) const
	{
		return Vec3d(vVector.x + x, vVector.y + y, vVector.z + z);
	}

	Vec3d operator-(const Vec3d vVector) const
	{
		return Vec3d(x - vVector.x, y - vVector.y, z - vVector.z);
	}

	Vec3d operator*(const float num) const
	{
		return Vec3d(x * num, y * num, z * num);
	}

	Vec3d operator*(const Vec3d v) const
	{
		return Vec3d(x * v.x, y * v.y, z * v.z);
	}

	Vec3d operator/(const double num) const
	{
		return Vec3d(x / num, y / num, z / num);
	}

	double& operator[](const int i)
	{
		return (double&)*((double*)(this)+i);
	}

	inline void set(const double* values)
	{
		x = values[0];
		y = values[1];
		z = values[2];
	}

	void transform(const Matrix& m);
	void transform3(const Matrix& m);
};

bool Close(Vec3d a, Vec3d b, double eps=CLOSE_EPSILON);

#endif
