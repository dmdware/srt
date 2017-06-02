

#ifndef SAVEENTITY_H
#define SAVEENTITY_H

#include "save.h"
#include "../platform.h"

//entity property ID's in save file
#define EPROP_CAM		0	//camera
#define EPROP_ETYPE		1	//entity type
#define EPROP_TYPES		2	//total number of properties

void SaveEntities(FILE* fp);
void ReadEntities(FILE* fp);

#endif