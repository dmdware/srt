


#ifndef BRUSHSIDE_H
#define BRUSHSIDE_H

#include "../math/vec3f.h"
#include "../math/plane3f.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../render/model.h"
#include "../render/vertexarray.h"

//tex coord equation - not used
class TexCEq
{
	float m_rot;	//degrees
	float m_scale[2];	//default 0.1, world to tex coordinates
	float m_offset[2];	//in world distances
};

class CutBrushSide;

class BrushSide
{
public:
	Plane3f m_plane;
	VertexArray m_drawva;
	unsigned int m_diffusem;
	unsigned int m_specularm;
	unsigned int m_normalm;
	//unsigned int m_ownerm;	//team colour map
	int m_ntris;
	Triangle2* m_tris;
	Plane3f m_tceq[2];	//tex coord uv equations
	Vec3f m_centroid;
	std::list<Vec3f> m_sideverts;
	
	BrushSide(const BrushSide& original);
	BrushSide& operator=(const BrushSide &original);
	BrushSide();
	BrushSide(Vec3f normal, Vec3f point);
	~BrushSide();
	void makeva();
	void vafromcut(CutBrushSide* cutside);
	void usedifftex();
	void usespectex();
	void usenormtex();
	void useteamtex();
	void gentexeq();	//fit texture to face
	void fittex();
	void remaptex();
};

Vec3f PlaneCrossAxis(Vec3f normal);

#endif