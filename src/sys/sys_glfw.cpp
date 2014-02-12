#include <cstdio>

#include <map>
#include <cstdlib>
#include <string>
#include <cassert>
#include <cstring>
#include <time.h>
#include <GL/glew.h>
#include <GL/glfw.h>

#ifdef _WIN32
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "GLFW_276.lib")
#pragma comment(lib, "GLEW_190.lib")
#pragma comment(lib, "portaudio_x64.lib")
#endif

#include "../defs.h"

bool g_ConsoleOn;

static GLuint VertexArrayID = 0;

static char cheatbuf[16];
static int cheatptr;


// key mapping
static std::map<std::string, int> keyMap;

#define MOUSE_BUTTON_BASE		(GLFW_KEY_LAST + 1)
#define MOUSE_WHEEL_BASE		(MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_LAST + 1)
#define NUM_KEYS				(MOUSE_WHEEL_BASE + 2)

static void BuildMap() {
	keyMap["space"] = GLFW_KEY_SPACE;
	keyMap["esc"] = GLFW_KEY_ESC;
	keyMap["f1"] = GLFW_KEY_F1;
	keyMap["f2"] = GLFW_KEY_F2;
	keyMap["f3"] = GLFW_KEY_F3;
	keyMap["f4"] = GLFW_KEY_F4;
	keyMap["f5"] = GLFW_KEY_F5;
	keyMap["f6"] = GLFW_KEY_F6;
	keyMap["f7"] = GLFW_KEY_F7;
	keyMap["f8"] = GLFW_KEY_F8;
	keyMap["f9"] = GLFW_KEY_F9;
	keyMap["f10"] = GLFW_KEY_F10;
	keyMap["f11"] = GLFW_KEY_F11;
	keyMap["f12"] = GLFW_KEY_F12;
	keyMap["f13"] = GLFW_KEY_F13;
	keyMap["f14"] = GLFW_KEY_F14;
	keyMap["f15"] = GLFW_KEY_F15;
	keyMap["f16"] = GLFW_KEY_F16;
	keyMap["f17"] = GLFW_KEY_F17;
	keyMap["f18"] = GLFW_KEY_F18;
	keyMap["f19"] = GLFW_KEY_F19;
	keyMap["f20"] = GLFW_KEY_F20;
	keyMap["f21"] = GLFW_KEY_F21;
	keyMap["f22"] = GLFW_KEY_F22;
	keyMap["f23"] = GLFW_KEY_F23;
	keyMap["f24"] = GLFW_KEY_F24;
	keyMap["f25"] = GLFW_KEY_F25;
	keyMap["up"] = GLFW_KEY_UP;
	keyMap["down"] = GLFW_KEY_DOWN;
	keyMap["left"] = GLFW_KEY_LEFT;
	keyMap["right"] = GLFW_KEY_RIGHT;
	keyMap["shift"] = GLFW_KEY_LSHIFT;
	keyMap["lshift"] = GLFW_KEY_LSHIFT;
	keyMap["rshift"] = GLFW_KEY_RSHIFT;
	keyMap["lctrl"] = GLFW_KEY_LCTRL;
	keyMap["rctrl"] = GLFW_KEY_RCTRL;
	keyMap["lalt"] = GLFW_KEY_LALT;
	keyMap["ralt"] = GLFW_KEY_RALT;
	keyMap["tab"] = GLFW_KEY_TAB;
	keyMap["enter"] = GLFW_KEY_ENTER;
	keyMap["backspace"] = GLFW_KEY_BACKSPACE;
	keyMap["insert"] = GLFW_KEY_INSERT;
	keyMap["del"] = GLFW_KEY_DEL;
	keyMap["pageup"] = GLFW_KEY_PAGEUP;
	keyMap["pagedown"] = GLFW_KEY_PAGEDOWN;
	keyMap["home"] = GLFW_KEY_HOME;
	keyMap["end"] = GLFW_KEY_END;
	keyMap["kp0"] = GLFW_KEY_KP_0;
	keyMap["kp1"] = GLFW_KEY_KP_1;
	keyMap["kp2"] = GLFW_KEY_KP_2;
	keyMap["kp3"] = GLFW_KEY_KP_3;
	keyMap["kp4"] = GLFW_KEY_KP_4;
	keyMap["kp5"] = GLFW_KEY_KP_5;
	keyMap["kp6"] = GLFW_KEY_KP_6;
	keyMap["kp7"] = GLFW_KEY_KP_7;
	keyMap["kp8"] = GLFW_KEY_KP_8;
	keyMap["kp9"] = GLFW_KEY_KP_9;
	keyMap["kpdivide"] = GLFW_KEY_KP_DIVIDE;
	keyMap["kpmultiply"] = GLFW_KEY_KP_MULTIPLY;
	keyMap["kpsubtract"] = GLFW_KEY_KP_SUBTRACT;
	keyMap["kpadd"] = GLFW_KEY_KP_ADD;
	keyMap["kpdecimal"] = GLFW_KEY_KP_DECIMAL;
	keyMap["kpequal"] = GLFW_KEY_KP_EQUAL;
	keyMap["kpenter"] = GLFW_KEY_KP_ENTER;
	keyMap["kpnumlock"] = GLFW_KEY_KP_NUM_LOCK;
	keyMap["capslock"] = GLFW_KEY_CAPS_LOCK;
	keyMap["scrolllock"] = GLFW_KEY_SCROLL_LOCK;
	keyMap["pause"] = GLFW_KEY_PAUSE;
	keyMap["lsuper"] = GLFW_KEY_LSUPER;
	keyMap["rsuper"] = GLFW_KEY_RSUPER;
	keyMap["menu"] = GLFW_KEY_MENU;

	keyMap["mouse1"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_1;
	keyMap["mouse2"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_2;
	keyMap["mouse3"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_3;
	keyMap["mouse4"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_4;
	keyMap["mouse5"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_5;
	keyMap["mouse6"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_6;
	keyMap["mouse7"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_7;
	keyMap["mouse8"] = MOUSE_BUTTON_BASE + GLFW_MOUSE_BUTTON_8;
	
	keyMap["mousewheelup"]   = MOUSE_WHEEL_BASE + 0;
	keyMap["mousewheeldown"] = MOUSE_WHEEL_BASE + 1;
}

int SYS_GetKeyNumber(const char *name) {
	assert(name != NULL);
	// check if printable char
	if (name[0] != '\0' && name[1] == '\0') {
		int v = name[0];
		if (v < ' ' || v > 127) {
			return -1;
		}
		if (v >= 'a' && v <= 'z') {
			return v - ('a' - 'A');
		}
		return v;
	}
    std::map<std::string, int>::iterator it;
	it = keyMap.find(name);
    if (it == keyMap.end()) {
        return -1;
    }
    return keyMap[name];
}

int SYS_GetNumKeys() {
	return NUM_KEYS;
	// ==== no joy!!! number of joy buttons = GLFW_JOYSTICK_LAST + 1;//
}


void SYS_Init() {
	srand((int)time(NULL));
	rs.xres = 0;
	rs.yres = 0;
	BuildMap();
	rs.on = false;
}

void SYS_GetInitialMousePos(int *x, int *y) {
	glfwGetMousePos(x, y);
}

bool SYS_IsRunning() {
	if (rs.on) {
		if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS ||
		   !glfwGetWindowParam(GLFW_OPENED)) {
			   return false;
		}
	}
	return !g_QuitFlag;
}

void SYS_CloseVideo()
{
	if (!rs.on) {
		return;
	}
	if (rs.coreGL) {
		glDeleteVertexArrays(1, &VertexArrayID);
		VertexArrayID = 0;
	}
	glfwTerminate();
	rs.on = false;
}

void SYS_Cleanup()
{
	SYS_CloseVideo();
}

static void check_cheats() {
	char cheat[sizeof(cheatbuf) + 1];

	for (int i = 0; i < (int)sizeof(cheatbuf); i++) {
		cheat[i] = cheatbuf[(cheatptr - i - 1) % sizeof(cheatbuf)];
	}
	if (strncmp(cheat, "dqddi", 5) == 0) {
		con_printf("cheater!!!\n");
		cheatbuf[0] = 0;
		cheatptr = 1;
	}
	//AAAAAAAAAAA CHEAT NO WORK
}

static void cheat_add(int ch) {
	cheatbuf[cheatptr] = ch;
	cheatptr = (cheatptr + 1 ) % sizeof(cheatbuf);
	if (ch != 0) {
		check_cheats();
	}
}

static void GLFWCALL WindowSizeCB(int w, int h) {
    rs.xres = w;
    rs.yres = h;
	RE_VidconResize();
}

static void GLFWCALL KeyCB(int ch, int action) {
	if (action != GLFW_PRESS) {
		if (!g_ConsoleOn || ch == 258) { // F1 always passes even in console
			CL_InputEvent(ch, false);
		}
		//con_printf("depressing key %d\n", ch);
        return;
    }
	// Check pressing F1
    if (ch == 258) { // F1
		// Toggle console
        g_ConsoleOn = !g_ConsoleOn;
		if (g_ConsoleOn) {
			glfwEnable(GLFW_KEY_REPEAT);
		} else {
			glfwDisable(GLFW_KEY_REPEAT);
		}
		CL_InputEvent(ch, true);
		return;
    }
	//con_printf("pressing key %d\n", ch);
	if (g_ConsoleOn) {
		RE_VidconKeyPress(ch);
	} else {
		CL_InputEvent(ch, true);
	}
	//cheat_add(0);
}

static void GLFWCALL CharCB(int ch, int action) {
    if (action != GLFW_PRESS) {
		//con_printf("depressing char %d\n", ch);
        return;
    }
	//con_printf("pressing char %d\n", ch);
	if (g_ConsoleOn) {
		RE_VidconCharPress(ch);
	}
}

static void GLFWCALL MouseWheelCB(int pos) {
	static bool first = true;
	static int last;
	if (first) {
		last = 0;
	}
	if (pos >= last) {
		CL_InputEvent(MOUSE_WHEEL_BASE + 0, true);
	} else {
		CL_InputEvent(MOUSE_WHEEL_BASE + 1, true);
	}
	last = pos;
}

static void GLFWCALL MousePosCB(int x, int y) {
	CL_InputMouse(x, y);
}

static void GLFWCALL MouseButtonCB(int button, int action) {
    if (action == GLFW_PRESS) {
		CL_InputEvent(MOUSE_BUTTON_BASE + button, true);
	} else {
		CL_InputEvent(MOUSE_BUTTON_BASE + button, false);
	}
}

void SYS_PollEvents() {
	glfwPollEvents();
}

static bool startGLFW(bool core) {
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, core ? 3 : 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, core ? 3 : 1);
	if (core) {
		glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
	int mode = opt.fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW;
	if (!glfwOpenWindow(opt.xres, opt.yres, 0,0,0,0, 32,0, mode)) {
		glfwTerminate();
		return false;
	}
	if (!core) {
		rs.coreGL = false;
	}
	return true;
}

bool SYS_InitVideo()
{
	if (rs.on) {
		return true;
	}
	if (!glfwInit()) {
		SYS_ShowError("Failed to initialize GLFW\n");
		return false;
	}
	rs.coreGL = true;
	if (!opt.forceGL21) {
		if (!startGLFW(true)) {
            glfwInit();
			if (!startGLFW(false)) {
				return false;
			}
		}
	} else {
		if (!startGLFW(false)) {
			return false;
		}
	}

#if 0

	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	rs.coreGL = true;
	int mode = opt.fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW;
	//if (!glfwOpenWindow(video_w, video_h, 0,0,0,0, 32,0, mode)) {
	if (!glfwOpenWindow(opt.xres, opt.yres, 0,0,0,0, 32,0, mode)) {
		rs.coreGL = false;
		glfwTerminate();
		if (!glfwInit()) {
			return false;
		}
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
		glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);
		if (!glfwOpenWindow(opt.xres, opt.yres, 0,0,0,0, 32,0, mode)) {
			glfwTerminate();
			return false;
		}
	}
#endif
	// should change in callback but set it here anyway for precaution
	rs.xres = opt.xres;
	rs.yres = opt.yres;
	//con_printf("aaaa %d %d\n", rs.xres, rs.yres);
	if (rs.coreGL) {
		glewExperimental = true;
	}
	if (glewInit() != GLEW_OK) {
		SYS_ShowError("Failed to initialize GLEW");
		return false;
	}
	// Always reporting an error here. Clear it!
	glGetError();
	if (rs.coreGL) {
		//glGenVertexArrays(1, &VertexArrayID);
		//glBindVertexArray(VertexArrayID);
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);
	}
	glfwDisable(GLFW_AUTO_POLL_EVENTS);
    glfwSetWindowSizeCallback(WindowSizeCB);
	glfwSetKeyCallback(KeyCB);
	glfwSetCharCallback(CharCB);
	glfwSetMouseButtonCallback(MouseButtonCB);
	glfwSetMousePosCallback(MousePosCB);
	glfwSetMouseWheelCallback(MouseWheelCB);
    glfwDisable(GLFW_MOUSE_CURSOR);
	glfwSetWindowTitle("Game");
    int wx = opt.winpos % 2;
    int wy = opt.winpos / 2;
    glfwSetWindowPos(wx * (rs.xres + 16), wy * (rs.yres + 10));
	memset(cheatbuf, 0, sizeof(cheatbuf));
	cheatptr = 0;
	rs.on = true;

	// Console starts on
	glfwEnable(GLFW_KEY_REPEAT);
	g_ConsoleOn = true;



	return true;
}

void SYS_FlipScreenPage()
{
	if (!rs.on) {
		return;
	}
	glfwSwapBuffers();
}
