#include <cassert>
#include <cstring>
#include "../defs.h"

svEntity_t *SV_GetEntityAt(vec3_t pos) {
    for (int i = 0; i < MAX_GAME_ENTITIES; i++) {
        svEntity_t *e = &ss.entities[i];
        if (!e->used) {
            continue;
        }
		vec3_t minv;
		vec3_t maxv;

		M_VectorCopy(minv, e->origin);
		M_VectorCopy(maxv, e->origin);
		minv[0] -= 2.0f;
		minv[1] -= 2.0f;
		minv[2] -= 3.5f;
		maxv[0] += 2.0f;
		maxv[1] += 2.0f;
		maxv[2] += 3.5f;
		if (pos[0] < minv[0] || pos[1] < minv[1] || pos[2] < minv[2]) {
			continue;
		}
		if (pos[0] > maxv[0] || pos[1] > maxv[1] || pos[2] > maxv[2]) {
			continue;
		}
		return e;
    }
    return NULL;
}

svEntity_t *SV_SpawnEntity(int i) {
    if (!IS_VALID_ENTITY_ID(i)) {
        CON_ERR("invalid entity id");
        return NULL;
    }
	svEntity_t *e = &ss.entities[i];
	assert(!e->used);
    memset(e, 0, sizeof(*e));
    e->used = true;
    e->id = i;
	return e;
}

int SV_SpawnEntity() {
	// look for an empty entity id (not a client entity of course)
    for (int i = MAX_CLIENTS; i < MAX_GAME_ENTITIES; i++) {
        svEntity_t *e = &ss.entities[i];
        if (e->used) {
            continue;
        }
        SV_SpawnEntity(i);
        return i;
    }
    return INVALID_ENTITY_ID;
}

bool SV_RemoveEntity(int i) {
    if (!IS_VALID_ENTITY_ID(i)) {
        CON_ERR("invalid entity id");
        return false;
    }
    svEntity_t *e = &ss.entities[i];
    e->used = false;
    return true;
}

svEntity_t *SV_GetEntityById(int id) {
	assert(IS_VALID_ENTITY_ID(id));
	return &ss.entities[id];
}

void SV_WriteEntity(szb_t *msg, svEntity_t *e) {
	assert(msg != NULL);
	assert(e != NULL);
    szb_write16(msg, e->id);
    szb_writeAng8(msg, e->angles[0]);
    szb_writeAng8(msg, e->angles[1]);
    szb_writeAng8(msg, e->angles[2]);
    szb_writePos(msg, e->origin);
    szb_write16(msg, e->frame);
    szb_write16(msg, e->model);
}