#include <cmath>
#include <cassert>
#include <cstdio>
#include "../defs.h"

static void add_splash_f();
static void toggle_mlook_f();
static void forward_f();
static void back_f();
static void left_f();
static void right_f();
static void jump_f();
static void attack_f();

cvar_t *var_mlook;

void CL_PlayerInit() {
	var_mlook = cvar_register("mlook", "1");
	cfun_register(toggle_mlook_f, "toggle_mlook");
	cfun_register(forward_f, "+forward");
	cfun_register(back_f, "+back");
	cfun_register(left_f, "+left");
	cfun_register(right_f, "+right");
	cfun_register(jump_f, "+jump");
	cfun_register(attack_f, "+attack");
	cfun_register(add_splash_f, "add_splash");

	con_addlines(
			     "bind w +forward\n"
			     "bind s +back\n"
			     "bind a +left\n"
			     "bind d +right\n"
			     "bind space +jump\n"
			     "bind mouse1 +attack\n"
			     "bind m toggle_mlook\n"
			     "bind n add_splash\n");
}


#define MY_PI	3.14159265f

#define ATTACK_DELAY	300

static int lastMouseX;
static int lastMouseY;
static bool mouse_ok = false;

#define MOUSE_LINUX

void CL_PlayerMove() {
	if (!cs.spawned) {
		return;
	}
	clEntity_t *e = &cs.player.entity;

    #ifdef MOUSE_LINUX

	float dx = float(cs.mouseX - cs.mouseXprev) * 0.4f;
	float dy = float(cs.mouseY - cs.mouseYprev) * 0.4f;

	cs.mouseXprev = cs.mouseX;
	cs.mouseYprev = cs.mouseY;

    #else

    if (!mouse_ok) {
        mouse_ok = true;
        SYS_GetInitialMousePos(&lastMouseX, &lastMouseY);
    }
    int currMouseX, currMouseY;
    SYS_GetInitialMousePos(&currMouseX, &currMouseY);
    
	float dx = float(currMouseX - lastMouseX) * 0.4f;
	float dy = float(currMouseY - lastMouseY) * 0.4f;

    lastMouseX = currMouseX;
    lastMouseY = currMouseY;

    #endif

	if (var_mlook->integer != 0) {
		e->angles[ANG_YAW] -= dx;
		e->angles[ANG_PITCH] += dy;
		if (e->angles[ANG_PITCH] > 80.0f) {
			e->angles[ANG_PITCH] = 80.0f;
		}
		if (e->angles[ANG_PITCH] < -80.0f) {
			e->angles[ANG_PITCH] = -80.0f;
		}
	}


	//con_printf("x=%d(%d) y=%d(%d) ang = %f\n", cs.mouseX, cs.mouseXprev, cs.mouseY, cs.mouseYprev, e->angles[ANG_YAW]);

	float ang_rad = e->angles[ANG_YAW] * MY_PI / 180.0f;
    float ang_sin = sin(ang_rad);
    float ang_cos = cos(ang_rad);
	float walk_step = float(g_FrameDelta) * 0.01f;
	cs.player.moving = false;

	if (cs.player.forward) {
        e->origin[0] += ang_cos * walk_step;
        e->origin[1] += ang_sin * walk_step;
		cs.player.moving = true;
    }
    if (cs.player.back) {
        e->origin[0] -= ang_cos * walk_step;
        e->origin[1] -= ang_sin * walk_step;
		cs.player.moving = true;
    }
    if (cs.player.left) {
        e->origin[0] -= ang_sin * walk_step;
        e->origin[1] += ang_cos * walk_step;
		cs.player.moving = true;
    }
    if (cs.player.right) {
        e->origin[0] += ang_sin * walk_step;
        e->origin[1] -= ang_cos * walk_step;
		cs.player.moving = true;
    }


	if (cs.player.attackDelay > 0) {
		cs.player.attackDelay -= g_FrameDelta;
		if (cs.player.attackDelay < 0) {
			cs.player.attackDelay = 0;
		}
	}

	if (cs.player.attack && cs.player.attackDelay == 0) {
		cs.player.sendAttack = true;
		//con_printf("vai atacar\n");
	}

	cs.viewAngles[0] = e->angles[0];
	cs.viewAngles[1] = e->angles[1];
	cs.viewAngles[2] = e->angles[2];
	cs.viewOrigin[0] = e->origin[0];
	cs.viewOrigin[1] = e->origin[1];
	cs.viewOrigin[2] = e->origin[2];



}

void CL_PlayerCleanup() {
}

void toggle_mlook_f() {
	if (!cs.spawned) {
		return;
	}
	cvar_set(var_mlook, var_mlook->integer ? 0 : 1);
}

void add_splash_f() {
	if (!cs.spawned) {
		return;
	}
	clEntity_t *e = &cs.player.entity;
	RE_AddSplash(rs.targetOrigin);//e->origin);

}

void forward_f() {
	if (!cs.spawned) {
		return;
	}
	cs.player.forward = cfun_argdown();
}

void back_f() {
	if (!cs.spawned) {
		return;
	}
	cs.player.back = cfun_argdown();
}

void left_f() {
	if (!cs.spawned) {
		return;
	}
	cs.player.left = cfun_argdown();
}

void right_f() {
	if (!cs.spawned) {
		return;
	}
	cs.player.right = cfun_argdown();
}

void jump_f() {
	if (!cs.spawned) {
		return;
	}
	cs.player.jump = cfun_argdown();
}

void attack_f() {
	if (!cs.spawned) {
		return;
	}
	cs.player.attack = cfun_argdown();
}


// =====
