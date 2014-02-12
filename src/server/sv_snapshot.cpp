#include <cassert>
#include <cstring>
#include "../defs.h"

void SV_BuildSnapshot(szb_t *msg, svClient_t *c) {
	assert(msg != NULL);
	assert(c != NULL);
    szb_write8(msg, S2C_SNAPSHOT);
    for (int i = 0; i < MAX_GAME_ENTITIES; i++) {
		svEntity_t *e = SV_GetEntityById(i);
        if (!e->used) {
            continue;
        }
        SV_WriteEntity(msg, e);
    }
    szb_write16(msg, INVALID_ENTITY_ID);


}