#include <stdlib.h>

static const char* splashes[] = {
	"heloo there!",
	"there is no box",
	"out of box?",
	"suspicious piece",
	"peacebox",
	"nice art!",
	"fall into pixels!",
	"64 is the limit!",
	"no one will read this",
	"that's unfortunate",
	":P",
	":3",
	"((char*)NULL)[10]"
};

static int splash_index = 0;

const char* getSplash() {
	return splashes[splash_index];	
}

void        rngSplash() {
	splash_index = rand() % (sizeof(splashes)/sizeof(char*));
}
