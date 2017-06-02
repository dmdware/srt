

#ifndef BORDER_H
#define BORDER_H

#include "../math/vec2i.h"

//territory per tile
extern signed char* g_border;

void DrawBords(Vec2i tmin, Vec2i tmax);

#endif