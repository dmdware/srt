

#ifndef MODELHOLDER_H
#define MODELHOLDER_H

#include "../render/model.h"
#include "../platform.h"

class ModelHolder
{
public:
	int model;
	Vec3f rotdegrees;
	Vec3f translation;
	Vec3f scale;
	Vec3f absmin;
	Vec3f absmax;
	Matrix rotationmat;
	VertexArray* frames;
	int nframes;

	ModelHolder();
	ModelHolder(int model, Vec3f pos);
	~ModelHolder();
	ModelHolder(const ModelHolder& original);
	ModelHolder& operator=(const ModelHolder &original);

	void retransform();
	void regennormals();
	void destroy();

	Vec3f traceray(Vec3f line[]);
};

extern std::list<ModelHolder> g_modelholder;

void FreeModelHolders();
void DrawModelHolders();
void DrawModelHoldersDepth();

#endif
