#ifndef SAVEMAP_H
#define SAVEMAP_H

#include "edmap.h"

#define MAP_TAG			{'P','2','M'}
#define MAP_VERSION		10

class Map
{
public:
	std::list<Brush> m_brush;
	std::list<ModelHolder> modelholder;
};

extern Map g_map;

float ConvertHeight(unsigned char brightness);
void FreeMap();
bool LoadMap(const char* relative);
bool SaveMap(const char* relative);

#endif
