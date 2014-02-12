#include <GL/glew.h>
#include "../defs.h"

refState_t rs;

void RE_ClearBuffers() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RE_ResetState() {
	// View port
    glViewport(0, 0, rs.xres, rs.yres);

	// Clear color
    glClearColor(0.0f, 0.4f, 0.0f, 0.0f);

    // Depth test
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Backface Culling
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    // Texture Mapping
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Blending
    glDisable(GL_BLEND);
    
    // Program
    glUseProgram(0);
}

void RE_PrintGLErrors(int line, const char *filename) {
	if (!rs.on) {
		return;
	}
	GLenum n;
	while ((n = glGetError()) != GL_NO_ERROR) {
		const char *str;

        str = "";
		switch(n) {
		case GL_INVALID_ENUM:
			str = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			str = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			str = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			str = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_OUT_OF_MEMORY:
			str = "GL_OUT_OF_MEMORY";
			break;
		case GL_STACK_UNDERFLOW:
			str = "GL_STACK_UNDERFLOW";
			break;
		case GL_STACK_OVERFLOW:
			str = "GL_STACK_OVERFLOW";
			break;
		}
		if (filename != NULL) {
			con_printf("OpenGL error at %s, %d: %s\n", filename, line, str);
		} else {
			con_printf("OpenGL error: %s\n", str);
		}
	}
}

float RE_GetCenterZ() {
	float depth;
	glReadPixels(rs.xres / 2, rs.yres / 2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
	return depth;
}
