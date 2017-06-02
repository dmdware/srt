
#ifndef DECAL_H
#define DECAL_H

#include "../math/3dmath.h"
#include "../math/vec3f.h"

class CDecalType
{
public:
	unsigned int tex;
	float decay;
	float size;
};

enum DECAL{BLOODSPLAT, BULLETHOLE, DECAL_TYPES};
extern CDecalType g_decalT[DECAL_TYPES];

class CDecal
{
public:
	bool on;
	int type;
	Vec3f a, b, c, d;
	Vec3f lpos;
	float life;
};

#define DECALS 128
extern CDecal g_decal[DECALS];

void Decals();
void UpdateDecals();
void DrawDecals();
void PlaceDecal(int type, Vec3f pos, Vec3f norm);

#endif