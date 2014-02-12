#include <cassert>
#include <cstdio>
#include "../defs.h"

static void bind_f();
static void unbind_f();
static void unbindall_f();

// events::
// key
// mouse button
// mouse wheel

// mouse pos

static char **binds = NULL;
static int numKeys;

static void CL_Bind(int key, const char *cmd) {
	if (binds == NULL) {
		return;
	}
	if (binds[key] != NULL) {
		delete[] binds[key];
	}
	binds[key] = my_strdup(cmd);
}

static void CL_Unbind(int key) {
	if (binds == NULL) {
		return;
	}
	if (binds[key] != NULL) {
		delete[] binds[key];
		binds[key] = NULL;
	}
}

static void CL_UnbindAll() {
	if (binds == NULL) {
		return;
	}
	for (int i = 0; i < numKeys; i++) {
		char *cmd = binds[i];
		if (cmd != NULL) {
			delete[] cmd;
		}
		binds[i] = NULL;
	}
}

void CL_InputInit() {
	numKeys = SYS_GetNumKeys();
	binds = new char *[numKeys];
	for (int i = 0; i < numKeys; i++) {
		binds[i] = NULL;
	}
	cfun_register(bind_f, "bind");
	cfun_register(unbind_f, "unbind");
	cfun_register(unbindall_f, "unbindall");

	SYS_GetInitialMousePos(&cs.mouseX, &cs.mouseY);
	cs.mouseXprev = cs.mouseX;
	cs.mouseYprev = cs.mouseY;
}

void CL_InputCleanup() {
	if (binds == NULL) {
		return;
	}
	CL_UnbindAll();
	delete[] binds;
	binds = NULL;
}

void CL_Bind(const char *key, const char *cmd) {
	int i = SYS_GetKeyNumber(key);
	if (i < 0) {
		con_printf("unknown key: %s\n", key);
		return;
	}
	CL_Bind(i, cmd);
	//con_printf("try bind\n");
}

void CL_InputEvent(int key, bool down) {
	if (binds == NULL) {
		return;
	}
	if (key >= numKeys) {
		return;
	}
	if (binds[key] == NULL) {
		return;
	}
	if (!down) {
		if (binds[key][0] != '+') {
			return;
		}
		binds[key][0] = '-';
		con_addlines(binds[key]);
		//con_printf("I will try to type %s\n", binds[key]);
		binds[key][0] = '+';
		return;
	}
	con_addlines(binds[key]);
	//con_printf("I will try to type %s\n", binds[key]);
}

void CL_InputMouse(int x, int y) {
	cs.mouseXprev = cs.mouseX;
	cs.mouseYprev = cs.mouseY;
	cs.mouseX = x;
	cs.mouseY = y;
}

//void CL_MouseButton

// ===== console functions ======
void bind_f() {
	if (binds == NULL) {
		return;
	}

	int argc = cfun_argc();
	if (argc == 2 || argc == 3) {
		const char *keyname = cfun_args(1);
		int key = SYS_GetKeyNumber(keyname);
		if (key < 0) {
			con_printf("\"%s\" is not a valid key\n", keyname);
			return;
		}
		if (argc == 2) {
			if (binds[key] != NULL) {
				con_printf("\"%s\" = \"%s\"\n", keyname, binds[key]);
			} else {
				con_printf("\"%s\" is not bound\n", keyname);
			}
			return;
		}
		// argc should be 3 then
		CL_Bind(key, cfun_args(2));
		return;
	}
	con_printf("Usage: bind <key name> <command>\n");
}

void unbind_f() {
	int argc = cfun_argc();
	if (argc == 2) {
		const char *keyname = cfun_args(1);
		int key = SYS_GetKeyNumber(keyname);
		if (key < 0) {
			con_printf("\"%s\" is not a valid key\n", keyname);
			return;
		}
		CL_Unbind(key);
		return;
	}
	con_printf("Usage: unbind <key name>\n");
}

void unbindall_f() {
	CL_UnbindAll();
}

// =====
