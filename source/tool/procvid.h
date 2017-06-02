

#ifndef PROCVID_H
#define PROCVID_H

//#define MAXPIXSEARCH 24
#define MAXPIXSEARCH 24
#define QUADSZ	3
#define MINCONTRAST 3
//#define MINCONTRAST 21
//#define MINCONTRAST 11
#define MATCHCONTRAST	16
#define PIXMATCHCONTRAST	190

void YUVfromRGB(double& Y, double& U, double& V, const double R, const double G, const double B);
void ProcVid(const char* fullpath);
void ProcFrame2(LoadedTex* comptex, LoadedTex* colortex);

#endif