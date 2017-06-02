#ifndef VEC2S_H
#define VEC2S_H

// byte-align structures
#pragma pack(push, 1)

class Vec2s
{
public:
	short x, y;

	Vec2s()
	{
		x = y = 0;
	}

	Vec2s(short X, short Y)
	{
		x = X;
		y = Y;
	}

	Vec2s(const short* values)
	{
		set(values);
	}

	bool operator==(const Vec2s vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return true;

		return false;
	}

	bool operator!=(const Vec2s vVector) const
	{
		if(x == vVector.x && y == vVector.y)
			return false;

		return true;
	}

	Vec2s operator+(const Vec2s vVector) const
	{
		return Vec2s(vVector.x + x, vVector.y + y);
	}

	Vec2s operator-(const Vec2s vVector) const
	{
		return Vec2s(x - vVector.x, y - vVector.y);
	}

	Vec2s operator*(const short num) const
	{
		return Vec2s(x * num, y * num);
	}

	Vec2s operator*(const Vec2s v) const
	{
		return Vec2s(x * v.x, y * v.y);
	}

	Vec2s operator/(const short num) const
	{
		return Vec2s(x / num, y / num);
	}

	inline void set(const short* values)
	{
		x = values[0];
		y = values[1];
	}

	short& operator[](unsigned char in)
	{
		return *(short*)(((char*)this)+in);
	}
};

// Default alignment
#pragma pack(pop)

#endif
