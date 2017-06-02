

#ifndef ESTVIEW_H
#define ESTVIEW_H

class LoadedTex;

extern float g_xfov;
extern float g_yfov;

void EstRotTl(LoadedTex* comptex, LoadedTex* colortex, LoadedTex* colortex2, int fin);

#endif