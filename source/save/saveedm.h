


#include "../platform.h"

//#define EDMAP_VERSION		2.0f

//#define TAG_EDMAP		{'D', 'M', 'D', 'S', 'P'}	//DMD sprite project


#define EDMAP_VERSION		2.0f
//#define EDMAP_VERSION		3.0f	//new Brush::collapse() colva colsh etc

#define TAG_EDMAP		{'D', 'M', 'D', 'B', 'M'}	//I forgot what the B stands for

class EdMap;
class TexRef;
class Brush;
class ModelHolder;

void ReadBrush(FILE* fp, TexRef* texrefs, Brush* b);
void SaveBrush(FILE* fp, int* texrefs, Brush* b);
void SaveEdMap(const char* fullpath, EdMap* map);
bool LoadEdMap(const char* fullpath, EdMap* map);
void FreeEdMap(EdMap* map);
void ReadEdTexs(FILE* fp, TexRef** texrefs);
void SaveTexs(FILE* fp, int* texrefs, std::list<Brush>& brushes);
void SaveBrushes(FILE* fp, int* texrefs, std::list<Brush>* brushes);
void ReadBrushes(FILE* fp, TexRef* texrefs, std::list<Brush>* brushes);
void SaveModelHolder(FILE* fp, ModelHolder* pmh);
void ReadModelHolder(FILE* fp, ModelHolder* pmh);
void SaveModelHolders(FILE* fp, std::list<ModelHolder>& modelholders);
void ReadModelHolders(FILE* fp, std::list<ModelHolder>& modelholders);
void ReadEnts(FILE* fp);
void SaveEnts(FILE* fp);