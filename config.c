#include <stdlib.h>
#include "config.h"

struct chip *chip = NULL;

struct chip chips[] = {
	{ "at89c1051",	0x5b, 1024, 8, VCC5, VPP12, 100 },
	{ "at89c2051",	0x5b, 2048, 8, VCC5, VPP12, 100 },
	{ "at89c4051",	0x5b, 4096, 8, VCC5, VPP12, 100 },
	{ "at89c51",	0x20, 4096, 8, VCC5, VPP5, 200 },
	{ "at89c51-12",	0x20, 4096, 8, VCC5, VPP12, 200 },
	{ "at89c52",	0x20, 8192, 8, VCC5, VPP5, 200 },
	{ "at89c52-12",	0x20, 8192, 8, VCC5, VPP12, 200 },
	{ "at90s1200",	0x5d, 1024, 16, VCC12, VPP12, 200 },
	{ "at89s8252",	0x5c, 10240, 8, VCC12, VPP12, 200 },
	
	{ "at29c010",	'Z', 131072, 8, 160, 160, 0 },
	
	{ NULL, 0, 0, 0, 0, 0, 0 }
};
