#include <cassert>
#include <cstdio>
#include "../defs.h"

#include <string>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include "../defs.h"
//#include "luacon.h"

map_t *thismap = NULL;

void CL_GameInit() {
	MDL_Load("goblin");
	MDL_Load("@");

	thismap = MAP_Load("game.map");
    CON_OK();
}

void CL_GameCleanup() {
	MAP_Free(thismap);
	thismap = NULL;
}
