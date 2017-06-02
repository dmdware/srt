

#include "../platform.h"
#include "checksum.h"
#include "../utils.h"

//TO DO: units as circles collision

unsigned int AddCheck(unsigned int sum, unsigned int next)
{
	sum++;
	sum = sum ^ next;
	//sum = (sum << 1) + (sum >> 7);	//for unsigned char
	sum = (sum << 1) + (sum >> 31);	//for unsigned int
	//sum = (sum << 1) + (sum >> (sizeof(sum) * 8 - 1));	//automatically determine
	return sum;
}

unsigned int CheckSum(const char* fullpath)
{
	unsigned int checksum = 0;
	FILE* fp = fopen(fullpath, "rb");

	if(!fp)
		return checksum;

	while(!feof(fp))
	{
		unsigned char next;
		fread(&next, sizeof(unsigned char), 1, fp);
		checksum = AddCheck(checksum, next);
	}

	fclose(fp);

#if 0
	char msg[128];
	sprintf(msg, "check %u", checksum);
	InfoMess("c", msg);
#endif

	return checksum;
}