#define _CRT_SECURE_NO_WARNINGS

#include <cstring>
#include <cstdio>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "../defs.h"

#define NUM_SHADERS 3

static struct shader_t {
	GLuint ProgramID, MatrixID, TextureID, FromID, ToID, UVID, AmountID;
	GLuint LightAttenID, LightAmbientID;
	GLuint NFromID, NToID;
	GLuint ViewMatrixID;
	GLuint ModelMatrixID;
	GLuint LightID;
} shaders[NUM_SHADERS];

static int currShader = 0;
static bool useLights = false;
static cvar_t *var_re_md2shader;

//========================================================================================
static float anorms[][3] = {
{ -0.525731f,  0.000000f,  0.850651f }, 
{ -0.442863f,  0.238856f,  0.864188f }, 
{ -0.295242f,  0.000000f,  0.955423f }, 
{ -0.309017f,  0.500000f,  0.809017f }, 
{ -0.162460f,  0.262866f,  0.951056f }, 
{  0.000000f,  0.000000f,  1.000000f }, 
{  0.000000f,  0.850651f,  0.525731f }, 
{ -0.147621f,  0.716567f,  0.681718f }, 
{  0.147621f,  0.716567f,  0.681718f }, 
{  0.000000f,  0.525731f,  0.850651f }, 
{  0.309017f,  0.500000f,  0.809017f }, 
{  0.525731f,  0.000000f,  0.850651f }, 
{  0.295242f,  0.000000f,  0.955423f }, 
{  0.442863f,  0.238856f,  0.864188f }, 
{  0.162460f,  0.262866f,  0.951056f }, 
{ -0.681718f,  0.147621f,  0.716567f }, 
{ -0.809017f,  0.309017f,  0.500000f }, 
{ -0.587785f,  0.425325f,  0.688191f }, 
{ -0.850651f,  0.525731f,  0.000000f }, 
{ -0.864188f,  0.442863f,  0.238856f }, 
{ -0.716567f,  0.681718f,  0.147621f }, 
{ -0.688191f,  0.587785f,  0.425325f }, 
{ -0.500000f,  0.809017f,  0.309017f }, 
{ -0.238856f,  0.864188f,  0.442863f }, 
{ -0.425325f,  0.688191f,  0.587785f }, 
{ -0.716567f,  0.681718f, -0.147621f }, 
{ -0.500000f,  0.809017f, -0.309017f }, 
{ -0.525731f,  0.850651f,  0.000000f }, 
{  0.000000f,  0.850651f, -0.525731f }, 
{ -0.238856f,  0.864188f, -0.442863f }, 
{  0.000000f,  0.955423f, -0.295242f }, 
{ -0.262866f,  0.951056f, -0.162460f }, 
{  0.000000f,  1.000000f,  0.000000f }, 
{  0.000000f,  0.955423f,  0.295242f }, 
{ -0.262866f,  0.951056f,  0.162460f }, 
{  0.238856f,  0.864188f,  0.442863f }, 
{  0.262866f,  0.951056f,  0.162460f }, 
{  0.500000f,  0.809017f,  0.309017f }, 
{  0.238856f,  0.864188f, -0.442863f }, 
{  0.262866f,  0.951056f, -0.162460f }, 
{  0.500000f,  0.809017f, -0.309017f }, 
{  0.850651f,  0.525731f,  0.000000f }, 
{  0.716567f,  0.681718f,  0.147621f }, 
{  0.716567f,  0.681718f, -0.147621f }, 
{  0.525731f,  0.850651f,  0.000000f }, 
{  0.425325f,  0.688191f,  0.587785f }, 
{  0.864188f,  0.442863f,  0.238856f }, 
{  0.688191f,  0.587785f,  0.425325f }, 
{  0.809017f,  0.309017f,  0.500000f }, 
{  0.681718f,  0.147621f,  0.716567f }, 
{  0.587785f,  0.425325f,  0.688191f }, 
{  0.955423f,  0.295242f,  0.000000f }, 
{  1.000000f,  0.000000f,  0.000000f }, 
{  0.951056f,  0.162460f,  0.262866f }, 
{  0.850651f, -0.525731f,  0.000000f }, 
{  0.955423f, -0.295242f,  0.000000f }, 
{  0.864188f, -0.442863f,  0.238856f }, 
{  0.951056f, -0.162460f,  0.262866f }, 
{  0.809017f, -0.309017f,  0.500000f }, 
{  0.681718f, -0.147621f,  0.716567f }, 
{  0.850651f,  0.000000f,  0.525731f }, 
{  0.864188f,  0.442863f, -0.238856f }, 
{  0.809017f,  0.309017f, -0.500000f }, 
{  0.951056f,  0.162460f, -0.262866f }, 
{  0.525731f,  0.000000f, -0.850651f }, 
{  0.681718f,  0.147621f, -0.716567f }, 
{  0.681718f, -0.147621f, -0.716567f }, 
{  0.850651f,  0.000000f, -0.525731f }, 
{  0.809017f, -0.309017f, -0.500000f }, 
{  0.864188f, -0.442863f, -0.238856f }, 
{  0.951056f, -0.162460f, -0.262866f }, 
{  0.147621f,  0.716567f, -0.681718f }, 
{  0.309017f,  0.500000f, -0.809017f }, 
{  0.425325f,  0.688191f, -0.587785f }, 
{  0.442863f,  0.238856f, -0.864188f }, 
{  0.587785f,  0.425325f, -0.688191f }, 
{  0.688191f,  0.587785f, -0.425325f }, 
{ -0.147621f,  0.716567f, -0.681718f }, 
{ -0.309017f,  0.500000f, -0.809017f }, 
{  0.000000f,  0.525731f, -0.850651f }, 
{ -0.525731f,  0.000000f, -0.850651f }, 
{ -0.442863f,  0.238856f, -0.864188f }, 
{ -0.295242f,  0.000000f, -0.955423f }, 
{ -0.162460f,  0.262866f, -0.951056f }, 
{  0.000000f,  0.000000f, -1.000000f }, 
{  0.295242f,  0.000000f, -0.955423f }, 
{  0.162460f,  0.262866f, -0.951056f }, 
{ -0.442863f, -0.238856f, -0.864188f }, 
{ -0.309017f, -0.500000f, -0.809017f }, 
{ -0.162460f, -0.262866f, -0.951056f }, 
{  0.000000f, -0.850651f, -0.525731f }, 
{ -0.147621f, -0.716567f, -0.681718f }, 
{  0.147621f, -0.716567f, -0.681718f }, 
{  0.000000f, -0.525731f, -0.850651f }, 
{  0.309017f, -0.500000f, -0.809017f }, 
{  0.442863f, -0.238856f, -0.864188f }, 
{  0.162460f, -0.262866f, -0.951056f }, 
{  0.238856f, -0.864188f, -0.442863f }, 
{  0.500000f, -0.809017f, -0.309017f }, 
{  0.425325f, -0.688191f, -0.587785f }, 
{  0.716567f, -0.681718f, -0.147621f }, 
{  0.688191f, -0.587785f, -0.425325f }, 
{  0.587785f, -0.425325f, -0.688191f }, 
{  0.000000f, -0.955423f, -0.295242f }, 
{  0.000000f, -1.000000f,  0.000000f }, 
{  0.262866f, -0.951056f, -0.162460f }, 
{  0.000000f, -0.850651f,  0.525731f }, 
{  0.000000f, -0.955423f,  0.295242f }, 
{  0.238856f, -0.864188f,  0.442863f }, 
{  0.262866f, -0.951056f,  0.162460f }, 
{  0.500000f, -0.809017f,  0.309017f }, 
{  0.716567f, -0.681718f,  0.147621f }, 
{  0.525731f, -0.850651f,  0.000000f }, 
{ -0.238856f, -0.864188f, -0.442863f }, 
{ -0.500000f, -0.809017f, -0.309017f }, 
{ -0.262866f, -0.951056f, -0.162460f }, 
{ -0.850651f, -0.525731f,  0.000000f }, 
{ -0.716567f, -0.681718f, -0.147621f }, 
{ -0.716567f, -0.681718f,  0.147621f }, 
{ -0.525731f, -0.850651f,  0.000000f }, 
{ -0.500000f, -0.809017f,  0.309017f }, 
{ -0.238856f, -0.864188f,  0.442863f }, 
{ -0.262866f, -0.951056f,  0.162460f }, 
{ -0.864188f, -0.442863f,  0.238856f }, 
{ -0.809017f, -0.309017f,  0.500000f }, 
{ -0.688191f, -0.587785f,  0.425325f }, 
{ -0.681718f, -0.147621f,  0.716567f }, 
{ -0.442863f, -0.238856f,  0.864188f }, 
{ -0.587785f, -0.425325f,  0.688191f }, 
{ -0.309017f, -0.500000f,  0.809017f }, 
{ -0.147621f, -0.716567f,  0.681718f }, 
{ -0.425325f, -0.688191f,  0.587785f }, 
{ -0.162460f, -0.262866f,  0.951056f }, 
{  0.442863f, -0.238856f,  0.864188f }, 
{  0.162460f, -0.262866f,  0.951056f }, 
{  0.309017f, -0.500000f,  0.809017f }, 
{  0.147621f, -0.716567f,  0.681718f }, 
{  0.000000f, -0.525731f,  0.850651f }, 
{  0.425325f, -0.688191f,  0.587785f }, 
{  0.587785f, -0.425325f,  0.688191f }, 
{  0.688191f, -0.587785f,  0.425325f }, 
{ -0.955423f,  0.295242f,  0.000000f }, 
{ -0.951056f,  0.162460f,  0.262866f }, 
{ -1.000000f,  0.000000f,  0.000000f }, 
{ -0.850651f,  0.000000f,  0.525731f }, 
{ -0.955423f, -0.295242f,  0.000000f }, 
{ -0.951056f, -0.162460f,  0.262866f }, 
{ -0.864188f,  0.442863f, -0.238856f }, 
{ -0.951056f,  0.162460f, -0.262866f }, 
{ -0.809017f,  0.309017f, -0.500000f }, 
{ -0.864188f, -0.442863f, -0.238856f }, 
{ -0.951056f, -0.162460f, -0.262866f }, 
{ -0.809017f, -0.309017f, -0.500000f }, 
{ -0.681718f,  0.147621f, -0.716567f }, 
{ -0.681718f, -0.147621f, -0.716567f }, 
{ -0.850651f,  0.000000f, -0.525731f }, 
{ -0.688191f,  0.587785f, -0.425325f }, 
{ -0.587785f,  0.425325f, -0.688191f }, 
{ -0.425325f,  0.688191f, -0.587785f }, 
{ -0.425325f, -0.688191f, -0.587785f }, 
{ -0.587785f, -0.425325f, -0.688191f }, 
{ -0.688191f, -0.587785f, -0.425325f }
};


class MDL_t {
public:
	std::string name;
    unsigned int vertexbuffer, normalbuffer, uvbuffer, elementbuffer, Texture;
    int numIndices;
    int frameSize;
    int numFrames;
	MDL_t() : vertexbuffer(0), normalbuffer(0), uvbuffer(0), elementbuffer(0), Texture(0),
		numIndices(0), frameSize(0), numFrames(0) {
	}
    ~MDL_t() {
		glDeleteBuffers(1, &uvbuffer);
		glDeleteBuffers(1, &normalbuffer);
		glDeleteBuffers(1, &vertexbuffer);
		glDeleteBuffers(1, &elementbuffer);
		glDeleteTextures(1, &Texture);
	}
};

static std::vector<MDL_t *> models;

static void MDL_LoadShaders() {
	shaders[0].ProgramID = RE_LoadShaders("md2.vert", "md2.frag");
	shaders[1].ProgramID  = RE_LoadShaders("md2phong.vert", "md2phong.frag");
	shaders[2].ProgramID  = RE_LoadShaders("md2gouraud.vert", "md2gouraud.frag");
	for (int i = 0; i < NUM_SHADERS; i++) {
		int ProgramID = shaders[i].ProgramID;

		shaders[i].ProgramID = ProgramID; 
		shaders[i].MatrixID = glGetUniformLocation(ProgramID, "MVP");
		shaders[i].TextureID = glGetUniformLocation(ProgramID, "tex");
		shaders[i].FromID = glGetAttribLocation(ProgramID, "pos_from");
		shaders[i].ToID = glGetAttribLocation(ProgramID, "pos_to");
		shaders[i].UVID = glGetAttribLocation(ProgramID, "uv");
		shaders[i].NFromID = glGetAttribLocation(ProgramID, "n_from");
		shaders[i].NToID = glGetAttribLocation(ProgramID, "n_to");
		shaders[i].AmountID = glGetUniformLocation(ProgramID, "amount");
		shaders[i].ViewMatrixID = glGetUniformLocation(ProgramID, "V");
		shaders[i].ModelMatrixID = glGetUniformLocation(ProgramID, "M");
		shaders[i].LightID = glGetUniformLocation(ProgramID, "LightPosition_worldspace");
		shaders[i].LightAttenID = glGetUniformLocation(ProgramID, "LightAtten");
		shaders[i].LightAmbientID = glGetUniformLocation(ProgramID, "LightAmbient");

	}

	currShader = 0;
}

static void MDL_FreeShaders() {
	for (int i = 0; i < NUM_SHADERS; i++) {
	    glDeleteProgram(shaders[i].ProgramID);
	}
}

MDL_t *MDL_Load(const char *path, float scale) {
	const char *tris_path = ASS_GetModelPath(path, "tris.md2");
	bool usedds = true;

    FILE *file = fopen(tris_path, "rb");
    if (file == NULL) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    if (filesize <= 0) {
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);
    char *data = new char[filesize];
    int r = int(fread(data, 1, filesize, file));
    fclose(file);
    struct header {
        int ident;
        int version;
        int skinwidth;
        int skinheight;
        int framesize;
        int num_skins;
        int num_xyz;
        int num_st;
        int num_tris;
        int num_glcmds;
        int num_frames;
        int ofs_skins;
        int ofs_st;
        int ofs_tris;
        int ofs_frames;
        int ofs_glcmds;
        int ofs_end;
    };
    header *hdr = (header *)data;
    struct frameheader {
        float scale[3];
        float translate[3];
        char name[16];
    };
    struct vertex {
        unsigned char v[3];
        unsigned char lightnormalindex;
    };

    std::vector<glm::vec3> V;
    std::vector<glm::vec3> N;
    std::vector<glm::vec2> UV;
    std::vector<unsigned short> E;

    con_printf("num frames = %d\n", hdr->num_frames);

	MDL_t *mdl = new MDL_t();

    for (int fn = 0; fn < hdr->num_frames; fn++) {
        frameheader *frame = (frameheader *)&data[hdr->ofs_frames + hdr->framesize * fn];
        //printf("name = (%s)\n", frame->name);
        vertex *fverts = (vertex *)(frame + 1);

        int next_index = 0;
        for (int tn = 0; tn < hdr->num_tris; tn++) {
            short *vt = (short *)&data[hdr->ofs_tris + tn * sizeof(short) * 6];
            for (int i = 0; i < 3; i++) {
                int v_index = vt[0+i];
                int uv_index = vt[3+i];

                float x = ((fverts[v_index].v[0] * frame->scale[0]) + frame->translate[0]) * scale;
                float y = ((fverts[v_index].v[1] * frame->scale[1]) + frame->translate[1]) * scale;
                float z = ((fverts[v_index].v[2] * frame->scale[2]) + frame->translate[2]) * scale;

				const float *np = anorms[fverts[v_index].lightnormalindex];

                glm::vec3 xyz = glm::vec3(x, y, z);
                V.push_back(xyz);
				glm::vec3 n = glm::vec3(np[0], np[1], np[2]);
				N.push_back(n);
				//("%f %f %f\n", n.x, n.y, n.z);

                if (fn == 0) {
                    short *st = (short *)&data[hdr->ofs_st + uv_index * sizeof(short) * 2];
					float vcoord = (float(st[1]) / hdr->skinheight);
					if (!usedds) {
						vcoord = 1.0f - vcoord;
					}
                    glm::vec2 uv = glm::vec2(float(st[0]) / hdr->skinwidth, vcoord);
                    //glm::vec2 uv = glm::vec2(float(st[0]) / hdr->skinwidth, float(st[1]) / hdr->skinheight);
                    UV.push_back(uv);
                }
                if (fn == 0) {
                    E.push_back(next_index++);
                }
            }
        }
        if (fn == 0) {
            mdl->frameSize = int(V.size() * sizeof(V[0]));
        }
        //printf("got %d UV=%d V=%d E=%d indices\n", next_index, int(UV.size()), int(V.size()), int(E.size()));
    }
    mdl->numFrames = hdr->num_frames;
    delete[] data;

    glGenBuffers(1, &mdl->elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, E.size()*sizeof(E[0]), &E[0], GL_STATIC_DRAW);

    glGenBuffers(1, &mdl->vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mdl->vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, V.size()*sizeof(V[0]), &V[0], GL_STATIC_DRAW);

	glGenBuffers(1, &mdl->normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mdl->normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, N.size()*sizeof(N[0]), &N[0], GL_STATIC_DRAW);

    glGenBuffers(1, &mdl->uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mdl->uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, UV.size()*sizeof(UV[0]), &UV[0], GL_STATIC_DRAW);

	const char *tex_path = ASS_GetModelPath(path, usedds ? "skin.DDS" : "skin.tga");

	mdl->Texture = RE_LoadTexture(tex_path, TEX_LM | TEX_CLAMP);
    mdl->numIndices = int(E.size());
    //mdl->isLoaded = true;
	models.push_back(mdl);

	return mdl;
}

void MDL_Free(MDL_t *mdl) {
	assert(mdl != NULL);
	delete mdl;
}

void MDL_SetModelMatrix(const float *m) {
	if (useLights) {
		glUniformMatrix4fv(shaders[currShader].ModelMatrixID, 1, GL_FALSE, m);
	}
}

void MDL_SetViewMatrix(const float *m) {
	if (useLights) {
		glUniformMatrix4fv(shaders[currShader].ViewMatrixID, 1, GL_FALSE, m);
	}
}

void MDL_SetMVPMatrix(const float *m) {
	glUniformMatrix4fv(shaders[currShader].MatrixID, 1, GL_FALSE, m);
}

void MDL_SetLightPos(const float *p) {
	if (useLights) {
		glUniform3f(shaders[currShader].LightID, p[0], p[1], p[2]);
	}
}

void MDL_SetLightParams(float attenuation, float ambient) {
	if (useLights) {
		glUniform1f(shaders[currShader].LightAttenID, attenuation);
		glUniform1f(shaders[currShader].LightAmbientID, ambient);
	}
}

void MDL_Render(MDL_t *mdl, int frame1, int frame2) {
	assert(mdl != NULL);
	if (frame1 < 0) {
		frame1 = 0;
	}
	if (frame1 >= mdl->numFrames) {
		frame1 = mdl->numFrames - 1;
	}
	if (frame2 < 0) {
		frame2 = 0;
	}
	if (frame2 >= mdl->numFrames) {
		frame2 = mdl->numFrames - 1;
	}
    glBindBuffer(GL_ARRAY_BUFFER, mdl->vertexbuffer);
    glVertexAttribPointer(
        shaders[currShader].FromID,  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)(((char*)0)+(frame1*mdl->frameSize))      // array buffer offset
    );
    glBindBuffer(GL_ARRAY_BUFFER, mdl->vertexbuffer);
    glVertexAttribPointer(
        shaders[currShader].ToID,  // The attribute we want to configure
        3,                            // size
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)(((char*)0)+(frame2*mdl->frameSize))      // array buffer offset
    );
    glBindBuffer(GL_ARRAY_BUFFER, mdl->uvbuffer);
    glVertexAttribPointer(
        shaders[currShader].UVID,                   // The attribute we want to configure
        2,                            // size : U+V => 2
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );
	if (useLights) {

		glBindBuffer(GL_ARRAY_BUFFER, mdl->normalbuffer);
		glVertexAttribPointer(
			shaders[currShader].NFromID,  // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)(((char*)0)+(frame1*mdl->frameSize))      // array buffer offset
		);
		glBindBuffer(GL_ARRAY_BUFFER, mdl->normalbuffer);
		glVertexAttribPointer(
			shaders[currShader].NToID,  // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)(((char*)0)+(frame2*mdl->frameSize))      // array buffer offset
		);

	}
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mdl->elementbuffer);
    // Draw the triangles !
    glDrawElements(
        GL_TRIANGLES,      // mode
        mdl->numIndices,        // count
        GL_UNSIGNED_SHORT, // type
        (void*)0           // element array buffer offset
    );
}

void MDL_BeginRender(int shader, float amount) {
	//currShader = shader;
	currShader = var_re_md2shader->integer;
	if (currShader < 0) {
		currShader = 0;
	}
	if (currShader >= NUM_SHADERS) {
		currShader = NUM_SHADERS - 1;
	}
	if (currShader > 0) {
		useLights = true;
	} else {
		useLights = false;
	}

    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glUseProgram(shaders[currShader].ProgramID);
    glActiveTexture(GL_TEXTURE0);
	glUniform1i(shaders[currShader].TextureID, 0);
    glUniform1f(shaders[currShader].AmountID, amount);
	glEnableVertexAttribArray(shaders[currShader].FromID);
    glEnableVertexAttribArray(shaders[currShader].ToID);
    glEnableVertexAttribArray(shaders[currShader].UVID);
	if (useLights) {
		glEnableVertexAttribArray(shaders[currShader].NFromID);
		glEnableVertexAttribArray(shaders[currShader].NToID);
	}
}

void MDL_EndRender() {
	if (useLights) {
		glDisableVertexAttribArray(shaders[currShader].NToID);
		glDisableVertexAttribArray(shaders[currShader].NFromID);
	}
    glDisableVertexAttribArray(shaders[currShader].UVID);
    glDisableVertexAttribArray(shaders[currShader].ToID);
    glDisableVertexAttribArray(shaders[currShader].FromID);
    glFrontFace(GL_CCW);
}

void MDL_PrepareTexture(MDL_t *mdl) {
	assert(mdl != NULL);
    glBindTexture(GL_TEXTURE_2D, mdl->Texture);
}

int MDL_GetId(const char *name) {
	assert(name != NULL);
	for (int i = 0; i < (int)models.size(); i++) {
		if (models[i]->name == name) {
			return i;
		}
	}
	return INVALID_MODEL_ID;
}

MDL_t* MDL_Get(int id) {
	if (id < 0 || id >= (int)models.size()) {
		return NULL;
	}
	return models[id];
}

void MDL_Cleanup() {
	for (int i = 0; i < (int)models.size(); i++) {
		delete models[i];
	}
	models.clear();
	MDL_FreeShaders();
}

void MDL_Init() {
	var_re_md2shader = cvar_register("md2shad", "1");
	MDL_LoadShaders();
}
