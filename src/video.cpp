#include <cstdio>
#include <cstring>
#include <AntTweakBar.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "defs.h"
#include "camera.h"

double g_CoreGL;
static int video_w, video_h;
static TwBar *bar;         // Pointer to a tweak bar

static void TW_CALL CloneCameraCB(void *clientData)
{
    CAM_Clone();
}

static void TW_CALL NextCameraCB(void *clientData)
{
    CAM_Next();
}

static void TW_CALL PrevCameraCB(void *clientData)
{
    CAM_Previous();
}

static void InitCameraButtons()
{
    TwAddButton(bar, "cloneCam", CloneCameraCB, NULL, " label='Clone Camera'");
    TwAddButton(bar, "nextCam", NextCameraCB, NULL, " label='Next Camera'");
    TwAddButton(bar, "prevCam", PrevCameraCB, NULL, " label='Previous Camera'");
    TwAddVarRO(bar, "camId", TW_TYPE_INT32, &g_CameraInterface.id, " label='ID' ");
    TwAddVarRW(bar, "camX", TW_TYPE_FLOAT, &g_CameraInterface.pos.x, "label='X' step=0.1 ");
    TwAddVarRW(bar, "camY", TW_TYPE_FLOAT, &g_CameraInterface.pos.y, "label='Y' step=0.1 ");
    TwAddVarRW(bar, "camZ", TW_TYPE_FLOAT, &g_CameraInterface.pos.z, "label='Z' step=0.1 ");
    TwAddVarRW(bar, "rotX", TW_TYPE_FLOAT, &g_CameraInterface.rot.x, "label='RX' step=1  min=-360 max=360 ");
    TwAddVarRW(bar, "rotY", TW_TYPE_FLOAT, &g_CameraInterface.rot.y, "label='RY' step=1  min=-360 max=360 ");
    TwAddVarRW(bar, "rotZ", TW_TYPE_FLOAT, &g_CameraInterface.rot.z, "label='RZ' step=1  min=-360 max=360 ");


}

// Callback function called by GLFW when window size changes
static void GLFWCALL WindowSizeCB(int width, int height)
{
    video_w = width;
    video_h = height;
    // Set OpenGL viewport and camera
    glViewport(0, 0, width, height);

    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //gluPerspective(40, (double)width/height, 1, 10);
    //gluLookAt(-1,0,3, 0,0,0, 0,1,0);    
    
    // Send the new window size to AntTweakBar
    TwWindowSize(width, height);
}

static void InitUI()
{
    // Initialize AntTweakBar
    //TwInit(TW_OPENGL_CORE, NULL);
    TwInit(TW_OPENGL, NULL);
    // Create a tweak bar
    bar = TwNewBar("Tools");
    TwDefine(" Tools size='240 240' ");
    // Set GLFW event callbacks
    // - Redirect window size changes to the callback function WindowSizeCB
    glfwSetWindowSizeCallback(WindowSizeCB);
    // - Directly redirect GLFW mouse button events to AntTweakBar
    glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW);
    // - Directly redirect GLFW mouse position events to AntTweakBar
    glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
    // - Directly redirect GLFW mouse wheel events to AntTweakBar
    glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
    // - Directly redirect GLFW key events to AntTweakBar
    glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
    // - Directly redirect GLFW char events to AntTweakBar
    glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);
    // Add 'time' to 'bar': it is a read-only (RO) variable of type TW_TYPE_DOUBLE, with 1 precision digit
    //TwAddVarRO(bar, "MenorZ", TW_TYPE_FLOAT, &oMenorZ, " label='MenorZ' help='menorZ.' ");         
    //TwAddButton(bar, "mlook", mlookCB, NULL, " label='MLook' key=TAB ");
    InitCameraButtons();
}

bool VID_IsRunning()
{
    return glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS &&
           glfwGetWindowParam(GLFW_OPENED);
}

void VID_GetMatrix(float *m)
{
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    glm::mat4 Projection = glm::perspective(45.0f, float(video_w) / float(video_h), 0.1f, 100.0f);
    // Camera matrix
    //glm::mat4 View       = glm::lookAt(
    //                            glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
    //                            glm::vec3(0,0,0), // and looks at the origin
    //                            glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    //                       );
    // Model matrix : an identity matrix (model will be at the origin)
    //glm::mat4 Model      = glm::mat4(1.0f);
    // Our ModelViewProjection : multiplication of our 3 matrices
    //glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
    //glm::mat4 MVP        = Projection; // Remember, matrix multiplication is the other way around
    memcpy(m, &Projection[0][0], sizeof(Projection));
}

static void FindCoreGL()
{
    //video_w = w;
    //video_h = h;
    // Initialise GLFW
    glfwInit();
    glfwOpenWindow(16, 16, 0,0,0,0, 32,0, GLFW_WINDOW);
    //glfwSetWindowPos(w,  h);
    //printf("%s\n", glGetString(GL_VERSION));
    g_CoreGL = false;
    int v[3];
    glfwGetGLVersion(&v[0], &v[1], &v[2]);
    if (v[0] >= 3) {
        if (v[0] > 3 || v[1] >= 3) {
            g_CoreGL = true;
        }
    }
    glfwTerminate();
}

int VID_Init(int w, int h)
{
    FindCoreGL();
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    //glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
    if (g_CoreGL) {
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 3);
        glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    } else {
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
        glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 1);
    }

    // Open a window and create its OpenGL context
    if (!glfwOpenWindow(w, h, 0,0,0,0, 32,0, GLFW_WINDOW)) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    // set experimental??
    if (g_CoreGL) {
        glewExperimental = true; // Needed for core profile
    }
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    glfwSetWindowTitle("CG Cam");
    CAM_Init();
    InitUI();
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); 
    return 0;
}

void VID_Cleanup()
{
    CAM_Cleanup();
    TwTerminate();
    glfwTerminate();
}

void VID_BeginFrame()
{
    glClearColor(0.0f, 0.8f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void VID_EndFrame()
{
    //TwRefreshBar();
    TwDraw();
    glfwSwapBuffers();
}

int VID_GetCmd()
{
	int cmd = 0;
	if (glfwGetKey('W')) {
		cmd |= 1;
	}
	if (glfwGetKey('S')) {
		cmd |= 2;
	}
	if (glfwGetKey('A')) {
		cmd |= 4;
	}
	if (glfwGetKey('D')) {
		cmd |= 8;
	}
	if (glfwGetKey(GLFW_KEY_LSHIFT)) {
		cmd |= 16;
	}
	return cmd;
}

#if 0
=============================================
static bool mlook = false;
static int mlook_lastx = 0, mlook_lasty = 0;

void VID_GetMouseDelta(int& dx, int& dy)
{
	if (!mlook) {
		dx = 0;
		dy = 0;
		return;
	}
	int xpos, ypos;
	glfwGetMousePos(&xpos, &ypos);
	dx = xpos - mlook_lastx;
	dy = ypos - mlook_lasty;
	mlook_lastx = xpos;
	mlook_lasty = ypos;
}

static void TW_CALL mlookCB(void *clientData)
{ 
	mlook = !mlook;
	if (mlook) {
		glfwDisable(GLFW_MOUSE_CURSOR);
		glfwGetMousePos(&mlook_lastx, &mlook_lasty);
	} else {
		glfwEnable(GLFW_MOUSE_CURSOR);
	}
    // do something
	/*
	m.LoadCommands();
	m.GoToLOD(0);
	//m.GoToLOD(0);
	int maxlod = m.GetMaxLOD();
	var.lod = maxlod;
	m.GoToLOD(maxlod);
	TwBar *bar = VID_GetBar();
	TwSetParam(bar, "lod", "max", TW_PARAM_INT32, 1, &maxlod);
	*/
}



#endif

#define MAX_TEXTURES    16

static GLuint textures[MAX_TEXTURES];

void VID_LoadTexture(int ti, const char *tn)
{
    char tname[128];

    assert(ti >= 0 && ti < MAX_TEXTURES);
    sprintf(tname, "data/%s", tn);
    textures[ti] = loadBMP_custom(tname);
}

void VID_LoadTextures()
{
    memset(textures, 0, sizeof(textures));
    VID_LoadTexture(TEX_BASIC0, "uvtemplate.bmp");
}

void VID_FreeTextures()
{
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (textures[i] != 0) {
            glDeleteTextures(1, &textures[i]);
            textures[i] = 0;
        }
    }
}

#define MAX_SHADERS     16

struct shader_t {
    GLuint ProgramID;
    GLuint ids[MAX_SHADERIDS];
} shaders[MAX_SHADERS];

static int curr_si;

void VID_LoadShader(int si, const char *vs, const char *fs)
{
    char vname[128];
    char fname[128];

    assert(si >= 0 && si < MAX_SHADERS);
    sprintf(vname, "data/%s.vert", vs);
    sprintf(fname, "data/%s.frag", fs);
    shaders[si].ProgramID = LoadShaders(vname, fname);
}

void VID_SetShaderId(int si, int sid, const char *name)
{
    if (sid == SHADERID_MAT || sid == SHADERID_TEX) {
        shaders[si].ids[sid] = glGetUniformLocation(shaders[si].ProgramID, name);
    } else {
        shaders[si].ids[sid] = glGetAttribLocation(shaders[si].ProgramID, name);
    }
}

void VID_LoadShaders()
{
    memset(shaders, 0, sizeof(shaders));
    VID_LoadShader(SHADER_COLORTEX, "simple", "simple");
    VID_SetShaderId(SHADER_COLORTEX, SHADERID_POS, "vertexPosition_modelspace");
    VID_SetShaderId(SHADER_COLORTEX, SHADERID_COLOR, "vertexColor");
    VID_SetShaderId(SHADER_COLORTEX, SHADERID_UV, "vertexUV");
    VID_SetShaderId(SHADER_COLORTEX, SHADERID_TEX, "myTextureSampler");
    VID_SetShaderId(SHADER_COLORTEX, SHADERID_MAT, "MVP");
}

void VID_FreeShaders()
{
    for (int i = 0; i < MAX_SHADERS; i++) {
        if (shaders[i].ProgramID != 0) {
            glDeleteProgram(shaders[i].ProgramID);
            shaders[i].ProgramID = 0;
        }
    }
}

void VID_SetUniform(int sid, const void *m)
{
    if (sid == SHADERID_MAT) {
        glUniformMatrix4fv(shaders[curr_si].ids[SHADERID_MAT], 1, GL_FALSE, (const float *)m);
    } else if (sid == SHADERID_TEX) {
        glUniform1i(shaders[curr_si].ids[SHADERID_TEX], *((const int *)m));
    }
}

void VID_SetUniform(int sid, int n)
{
    if (sid == SHADERID_TEX) {
        glUniform1i(shaders[curr_si].ids[SHADERID_TEX], n);
    }
}

void VID_SelectShader(int si)
{
    curr_si = si;
    glUseProgram(shaders[curr_si].ProgramID);
}

void VID_SelectTexture(int ui, int ti)
{
    GLuint texu;
    switch(ui) {
        case 0: texu = GL_TEXTURE0; break;
        case 1: texu = GL_TEXTURE1; break;
        default: return;
    }
    glActiveTexture(texu);
    glBindTexture(GL_TEXTURE_2D, textures[ti]);
}

unsigned int VID_GetShaderId(int sid)
{
    return shaders[curr_si].ids[sid];
}
