


/*******************************************************
 * Copyright (C) 2015 DMD 'Ware <dmdware@gmail.com>
 * 
 * This file is part of States, Firms, & Households.
 * 
 * You are entitled to use this source code to learn.
 *
 * You are not entitled to duplicate or copy this source code 
 * into your own projects, commercial or personal, UNLESS you 
 * give credit.
 *
 *******************************************************/




#ifndef UNITMOVE_H
#define UNITMOVE_H

class Unit;

void MoveUnit(Unit* u);
bool UnitCollides(Unit* u, Vec2i cmpos, int utype);
bool CheckIfArrived(Unit* u);
void OnArrived(Unit* u);

#endif
