
#ifndef UNDOH_H
#define UNDOH_H

#include "../../bsp/brush.h"
#include "../../save/edmap.h"
#include "../../platform.h"
#include "../../save/modelholder.h"
#include "sesim.h"
#include "../../sim/entity.h"

class UndoH	//undo history
{
public:
	std::list<Brush> brushes;
	std::list<ModelHolder> modelholders;
	Ent entities[ENTITIES];

	UndoH(){}
	~UndoH();
};

#define MAX_UNDO		16

void LinkPrevUndo(UndoH* tosave=NULL);	//call this BEFORE the change is made
void LinkLatestUndo();	//called by undo itself
void WriteH(UndoH* writeto);
void Undo();
void Redo();
void ClearUndo();

#endif