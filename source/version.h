


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






#ifndef VERSION_H
#define VERSION_H

#define VERSTAT_PA	0	//pre-alpha
#define VERSTAT_A	1	//alpha
#define VERSTAT_B	2	//beta
#define VERSTAT_RC	3	//release candidate
#define VERSTAT_F	4	//final

extern const char* VERSTAT_STR[5];

unsigned long long MakeVer(unsigned char major, unsigned char featureset, unsigned char attempt, unsigned char status, unsigned char postrelease);
void ParseVer(unsigned long long number, unsigned char* major, unsigned char* featureset, unsigned char* attempt, unsigned char* status);
void VerStr(unsigned long long number, char* str);

#endif