#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>

#include "defs.h"

static GLuint Texture;
static GLuint TextureID;
static GLuint vertexbuffer;
static GLuint uvbuffer;
static GLuint programID;
static GLuint MatrixID;
static GLuint vertexPosition_modelspaceID;
static GLuint vertexUVID;

void MODEL_Init()
{
    programID = LoadShaders("data/TransformVertexShader.vertexshader", "data/TextureFragmentShader.fragmentshader");
    MatrixID = glGetUniformLocation(programID, "MVP");
    vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    vertexUVID = glGetAttribLocation(programID, "vertexUV");
    Texture = loadBMP_custom("data/uvtemplate.bmp");
    //Texture = loadDDS("uvtemplate.DDS");
    TextureID  = glGetUniformLocation(programID, "myTextureSampler");
    // Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
    // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
    static const GLfloat g_vertex_buffer_data[] = { 
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f, 1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f,-1.0f, 1.0f
    };

    // Two UV coordinatesfor each vertex. They were created withe Blender.
    static const GLfloat g_uv_buffer_data[] = { 
        0.000059f, 1.0f-0.000004f, 
        0.000103f, 1.0f-0.336048f, 
        0.335973f, 1.0f-0.335903f, 
        1.000023f, 1.0f-0.000013f, 
        0.667979f, 1.0f-0.335851f, 
        0.999958f, 1.0f-0.336064f, 
        0.667979f, 1.0f-0.335851f, 
        0.336024f, 1.0f-0.671877f, 
        0.667969f, 1.0f-0.671889f, 
        1.000023f, 1.0f-0.000013f, 
        0.668104f, 1.0f-0.000013f, 
        0.667979f, 1.0f-0.335851f, 
        0.000059f, 1.0f-0.000004f, 
        0.335973f, 1.0f-0.335903f, 
        0.336098f, 1.0f-0.000071f, 
        0.667979f, 1.0f-0.335851f, 
        0.335973f, 1.0f-0.335903f, 
        0.336024f, 1.0f-0.671877f, 
        1.000004f, 1.0f-0.671847f, 
        0.999958f, 1.0f-0.336064f, 
        0.667979f, 1.0f-0.335851f, 
        0.668104f, 1.0f-0.000013f, 
        0.335973f, 1.0f-0.335903f, 
        0.667979f, 1.0f-0.335851f, 
        0.335973f, 1.0f-0.335903f, 
        0.668104f, 1.0f-0.000013f, 
        0.336098f, 1.0f-0.000071f, 
        0.000103f, 1.0f-0.336048f, 
        0.000004f, 1.0f-0.671870f, 
        0.336024f, 1.0f-0.671877f, 
        0.000103f, 1.0f-0.336048f, 
        0.336024f, 1.0f-0.671877f, 
        0.335973f, 1.0f-0.335903f, 
        0.667969f, 1.0f-0.671889f, 
        1.000004f, 1.0f-0.671847f, 
        0.667979f, 1.0f-0.335851f
    };

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
}

void MODEL_Free()
{
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &TextureID);
}

void MODEL_Render()
{
    // Use our shader
    glUseProgram(programID);

    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    glm::mat4 MVP;
    VID_GetMatrix(&MVP[0][0]);
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to user Texture Unit 0
    glUniform1i(TextureID, 0);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(vertexPosition_modelspaceID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        vertexPosition_modelspaceID,  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(vertexUVID);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        vertexUVID,                   // The attribute we want to configure
        2,                            // size : U+V => 2
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexUVID);
}
