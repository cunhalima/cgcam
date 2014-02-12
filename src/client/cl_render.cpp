#include <cassert>
#include "../defs.h"

static void my_mix(vec3_t dst, const vec3_t a, const vec3_t b, float lerp) {
	dst[0] = a[0] * (1.0f - lerp) + b[0] * lerp;
	dst[1] = a[1] * (1.0f - lerp) + b[1] * lerp;
	dst[2] = a[2] * (1.0f - lerp) + b[2] * lerp;
}

static void M_AnglesLerp(vec3_t dst, const vec3_t a, const vec3_t b, float lerp) {
	for (int i = 0; i < 3; i++) {
		// prob: a = 350 b = 5       ::-345      adiff = 15      bdiff = -705
		//       a = 5   b = 350     :: 345		 adiff = 705     bdiff = -15
		float diff = b[i] - a[i];
		if (diff < -180.0f) {
			dst[i] = (a[i] - 360.0f) * (1.0f - lerp) + b[i] * lerp;
		} else if (diff > 180.0f) {
			dst[i] = a[i] * (1.0f - lerp) + (b[i] - 360.0f) * lerp;
		} else {
			dst[i] = a[i] * (1.0f - lerp) + b[i] * lerp;
		}
	}
}

static float CL_DoClientRender() {
	if (cs.snapA == NULL) {
		return 0.0f;
	}
	clSnapshot_t *prevSnap;
	clSnapshot_t *nextSnap;

	int delta;

	if (cs.simTime < cs.snapA->time) {
		prevSnap = nextSnap = cs.snapA;
		delta = 0;
	} else if (cs.simTime >= cs.snapA->time && cs.simTime < cs.snapB->time) {
		prevSnap = cs.snapA;
		nextSnap = cs.snapB;
		delta = cs.simTime - cs.snapA->time;
	} else if (cs.simTime >= cs.snapB->time && cs.simTime <= cs.snapC->time) {
		prevSnap = cs.snapB;
		nextSnap = cs.snapC;
		delta = cs.simTime - cs.snapB->time;
	} else {
		prevSnap = nextSnap = cs.snapC;
		delta = 0;
	}
	float lerp = float(delta) / float(SERVER_TIC);

	// add self entity 
	refEntity_t p;
	{
		clEntity_t *o = &cs.player.entity;
		p.angles[0] = o->angles[0];
		p.angles[1] = o->angles[1];
		p.angles[2] = o->angles[2];
		p.origin[0] = o->origin[0];
		p.origin[1] = o->origin[1];
		p.origin[2] = o->origin[2];
		p.model = o->model;
	}

	for (int i = 0; i < MAX_GAME_ENTITIES; i++) {
		clEntity_t *e1, *e2;

		e1 = &prevSnap->entities[i];
		if (!e1->used) {
			continue;
		}
		e2 = &nextSnap->entities[i];
		if (!e2->used) {
			e2 = e1;
		}
		//con_printf("AAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
		refEntity_t e;
		//e.angles[0] = 0.0f;//(e1->angles[0] + e2->angle[0]) / 2.0f;
		//e.angles[1] = e1->angles[1];
		//e.angles[2] = e1->angles[2];
		M_AnglesLerp(e.angles, e1->angles, e2->angles, lerp);
		my_mix(e.origin, e1->origin, e2->origin, lerp);
		///e.origin[0] = e1->origin[0];
		//e.origin[1] = e1->origin[1];
		//e.origin[2] = e1->origin[2];
		e.model = e1->model;
		e.frame1 = e1->frame;
		e.frame2 = e2->frame;

		// HACK
		if (e1->id == cs.player.entity.id) {
			continue;
		}

		if (e1->id == cs.player.entity.id) {
			p.frame1 = e.frame1;
			p.frame2 = e.frame2;
			RE_AddRefEntityToScene(&p);
			continue;
		}

		RE_AddRefEntityToScene(&e);
	}


	//int time = cs.currSnap->time

	//lerp = float(cs.simTime - cs.prevSnap->time) / 1000.0f;

	//int time = g_Time % 1000;
	//float ipos = float(time) / 1000.0f;

	return lerp;
}

void CL_RenderFrame() {
	RE_BeginScene();
	RE_ResetState();
	RE_ClearBuffers();
	T01_Frame();
	//CL_GameFrame();

	float lerp = CL_DoClientRender();

	RE_EndScene();




	RE_RenderScene(lerp);
	//T03_Frame();

	RE_VidconDraw();
	RE_TextRender();
	// Swap buffers
	//CON_ProcessEvents();


	SYS_FlipScreenPage();

}
