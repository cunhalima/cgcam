#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include "defs.h"

static GLuint vertexbuffer;
static GLuint colorbuffer;
static GLuint uvbuffer;

void MAP_Load()
{
    static const GLfloat g_vertex_buffer_data[] = { 
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
    };

    static const GLfloat g_color_buffer_data[] = { 
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
    };

    static const GLfloat g_uv_buffer_data[] = { 
        0.0f, 0.0f, 
        0.0f, 1.0f, 
        1.0f, 1.0f,
        1.0f, 0.0f,
    };

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
}

void MAP_Free()
{
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &colorbuffer);
    glDeleteBuffers(1, &uvbuffer);
}

void MAP_Render(const float *m)
{
    VID_SelectShader(SHADER_COLORTEX);
    VID_SetUniform(SHADERID_MAT, m);
    VID_SelectTexture(0, TEX_BASIC0);
    glEnableVertexAttribArray(VID_GetShaderId(SHADERID_POS));
    glEnableVertexAttribArray(VID_GetShaderId(SHADERID_COLOR));
    glEnableVertexAttribArray(VID_GetShaderId(SHADERID_UV));
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        VID_GetShaderId(SHADERID_POS),  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        VID_GetShaderId(SHADERID_COLOR),  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        VID_GetShaderId(SHADERID_UV),                   // The attribute we want to configure
        2,                            // size : U+V => 2
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );
    glDrawArrays(GL_TRIANGLES, 0, 3); // 12*3 indices starting at 0 -> 12 triangles
    glDisableVertexAttribArray(VID_GetShaderId(SHADERID_UV));
    glDisableVertexAttribArray(VID_GetShaderId(SHADERID_COLOR));
    glDisableVertexAttribArray(VID_GetShaderId(SHADERID_POS));
}
