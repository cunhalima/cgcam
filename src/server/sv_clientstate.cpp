#include <cassert>
#include <cstring>
#include "../defs.h"

void SV_ReadClientState(svClient_t *c, szb_t *msg) {
	assert(c != NULL);
	assert(msg != NULL);
	if (var_dbconn->integer != 0) {
		con_print("receiving client state\n");
	}
	//con_printf("server reading STATE\n");
	svEntity_t *e = SV_GetEntityById(c->id);
	svPlayer_t *p = SV_GetPlayerFromEntity(e);

	e->used = true;
	//e->id = c->id;
    e->angles[0] = szb_readAng8(msg);
    e->angles[1] = szb_readAng8(msg);
    e->angles[2] = szb_readAng8(msg);
    szb_readPos(msg, e->origin);
    e->model = szb_read16(msg);
	p->moving = szb_read8(msg) != 0;
	p->attack = szb_read8(msg) != 0;
	p->jump = szb_read8(msg) != 0;

	p->doAttack = szb_read8(msg) != 0;
	if (p->doAttack) {
	    szb_readPos(msg, p->target);
		p->targetEntity = SV_GetEntityAt(p->target);
		if (p->targetEntity != NULL) {
			SV_HitEntity(p->targetEntity);
		}
	}

	if (var_dbconn->integer != 0) {
		//con_printf("server got e %d x=%f ang=%f:\n", c->id, e->origin[0], e->angles[0]);
		//szb_print(msg);
	}
}