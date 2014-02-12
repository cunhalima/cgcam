#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <GL/glew.h>
//#include <GL/glfw.h>
#include "../defs.h"

#define MAP_SIZE 16
#define NUM_SHADERS 1

static bool usedds = true;

static struct shader_t {
	GLuint ProgramID, TextureID, VertexID, UVID, NormalID;
	GLuint MVPMatrixID, ViewMatrixID, ModelMatrixID;
	GLuint LightID;
} shaders[NUM_SHADERS];

static int currShader = 0;

static void MAP_LoadShaders() {
	shaders[0].ProgramID = RE_LoadShaders("map.vert", "map.frag");
	for (int i = 0; i < NUM_SHADERS; i++) {
		int ProgramID = shaders[i].ProgramID;

		shaders[i].ProgramID = ProgramID; 
		shaders[i].MVPMatrixID = glGetUniformLocation(ProgramID, "MVP");
		shaders[i].ViewMatrixID = glGetUniformLocation(ProgramID, "V");
		shaders[i].ModelMatrixID = glGetUniformLocation(ProgramID, "M");
		shaders[i].TextureID = glGetUniformLocation(ProgramID, "tex");
		shaders[i].VertexID = glGetAttribLocation(ProgramID, "vertex_ms");
		shaders[i].UVID = glGetAttribLocation(ProgramID, "uv_ts");
		shaders[i].NormalID = glGetAttribLocation(ProgramID, "normal_ms");
		shaders[i].LightID = glGetUniformLocation(ProgramID, "light_ws");
	}
	currShader = 0;
}

static void MAP_FreeShaders() {
	for (int i = 0; i < NUM_SHADERS; i++) {
	    glDeleteProgram(shaders[i].ProgramID);
	}
}

class map_t {
public:
	GLuint elementbuffer;
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint Texture;
	char tilemap[MAP_SIZE*MAP_SIZE];
	int numIndices;
	int tileAt(int j, int i) {
		if (i < 0 || j < 0 || i >= MAP_SIZE || j >= MAP_SIZE) {
			return 1;
		}
		return tilemap[i*MAP_SIZE+j];
	}
	void LoadTileMap(const char *tmap) {
		for (int p = 0; p < (int)sizeof(tilemap); p++) {
			char c = tmap[p];
			if (c == ' ') {
				c = 0;
			} else {
				c = 1;
			}
			int i = MAP_SIZE - (p / MAP_SIZE) - 1;
			int j = p % MAP_SIZE;
			tilemap[i*MAP_SIZE+j] = c;
		}
	}


};

static char defmap[] =
	"################"
	"#              #"
	"#              #"
	"# ######       #"
	"#     #        #"
	"#     #        #"
	"#     #        #"
	"#     #        #"
	"#    ######    #"
	"#              #"
	"#              #"
	"#    #         #"
	"#    #   #     #"
	"#    #####     #"
	"#              #"
	"################";


void addwall(map_t *map, std::vector<glm::vec3>& V, std::vector<glm::vec2>& UV,
			std::vector<unsigned short>& E, int j, int i, int normaldir) {
	assert(map != NULL);
#define HEIGHT 2
	static glm::vec3 points[] = {
		glm::vec3(0, 0, 0),
		glm::vec3(1, 0, 0),
		glm::vec3(1, 1, 0),
		glm::vec3(0, 1, 0),
		glm::vec3(0, 0, HEIGHT),
		glm::vec3(1, 0, HEIGHT),
		glm::vec3(1, 1, HEIGHT),
		glm::vec3(0, 1, HEIGHT),};
	static int indices[] = {
		0, 3, 7, 4,				// face east
		1, 0, 4, 5,				// face north
		2, 1, 5, 6,				// face west
		3, 2, 6, 7,				// face south
	};
	if (map->tileAt(j, i)) {
		return;
	}
	int wi = i, wj = j;
	int index = 0;
	if (normaldir == 0) {
		index = 0;
		wj--;
	} else if (normaldir == 90) {
		index = 4;
		wi--;
	} else if (normaldir == 180) {
		index = 8;
		wj++;
	} else if (normaldir == 270) {
		index = 12;
		wi++;
	}
	if (!map->tileAt(wj, wi)) {
		return;
	}
	float scale = 8.0f;
	glm::vec3 base = glm::vec3(j, i, 0);
	int e = (int)V.size();
	for (int c = 0; c < 4; c++) {
		//V.push_back(points[indices[index + c]] + base);
		glm::vec3 v = (points[indices[index + c]] + base) * scale;
		//con_printf("%f %f %f\n", v.x, v.y, v.z);
		V.push_back(v);
	}
	UV.push_back(glm::vec2(0, usedds ? HEIGHT : 0));
	UV.push_back(glm::vec2(1, usedds ? HEIGHT : 0));
	UV.push_back(glm::vec2(1, usedds ? 0 : HEIGHT));
	UV.push_back(glm::vec2(0, usedds ? 0 : HEIGHT));

	E.push_back(e + 0);
	E.push_back(e + 1);
	E.push_back(e + 2);
	E.push_back(e + 0);
	E.push_back(e + 2);
	E.push_back(e + 3);
}

map_t *MAP_Load(const char *path) {
	map_t *map = new map_t();

	map->LoadTileMap(defmap);

	std::vector<glm::vec3> V;
	std::vector<glm::vec2> UV;
	std::vector<unsigned short> E;

	/*

	V.push_back(glm::vec3(-1, -1, 0));
	V.push_back(glm::vec3(0.5f, -1, 0));
	V.push_back(glm::vec3(0, 1, 0));

	UV.push_back(glm::vec2(0, 0));
	UV.push_back(glm::vec2(1, 0));
	UV.push_back(glm::vec2(1, 1));

	E.push_back(0);
	E.push_back(1);
	E.push_back(2);
	*/

	///*
	for (int i = 0; i < MAP_SIZE; i++) {
		for (int j = 0; j < MAP_SIZE; j++) {
			addwall(map, V, UV, E, j, i, 0);
			addwall(map, V, UV, E, j, i, 90);
			addwall(map, V, UV, E, j, i, 180);
			addwall(map, V, UV, E, j, i, 270);
		}
	}
	//*/
	//addwall(map, V, UV, E, 1, 1, 0);
	//addwall(map, V, UV, E, 1, 1, 90);
	//addwall(map, V, UV, E, 1, 1, 180);
	//addwall(map, V, UV, E, 1, 1, 270);

	//addwall(map, V, UV, E, 0, 2, 270);

	map->numIndices = (int)E.size();

    glGenBuffers(1, &map->elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, map->elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, E.size()*sizeof(E[0]), &E[0], GL_STATIC_DRAW);

	glGenBuffers(1, &map->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, map->vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, V.size() * sizeof(V[0]), &V[0], GL_STATIC_DRAW);

	glGenBuffers(1, &map->uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, map->uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, UV.size() * sizeof(UV[0]), &UV[0], GL_STATIC_DRAW);

	const char *tex_path = ASS_GetPath(usedds ? "brick1.DDS" : "brick1.tga");

	map->Texture = RE_LoadTexture(tex_path, TEX_LM | TEX_REPEAT);

	return map;
}

void MAP_Free(map_t *map) {
	assert(map != NULL);
	glDeleteBuffers(1, &map->elementbuffer);
	glDeleteBuffers(1, &map->uvbuffer);
	glDeleteBuffers(1, &map->vertexbuffer);
	glDeleteTextures(1, &map->Texture);
}

void MAP_BeginRender() {
	glUseProgram(shaders[currShader].ProgramID);
}

void MAP_SetModelMatrix(const float *m) {
	glUniformMatrix4fv(shaders[currShader].ModelMatrixID, 1, GL_FALSE, m);
}

void MAP_SetViewMatrix(const float *m) {
	glUniformMatrix4fv(shaders[currShader].ViewMatrixID, 1, GL_FALSE, m);
}

void MAP_SetMVPMatrix(const float *m) {
	glUniformMatrix4fv(shaders[currShader].MVPMatrixID, 1, GL_FALSE, m);
}

void MAP_Render(map_t *map) {
	assert(map != NULL);
	// Use our shader
	//con_printf("%d\n", programID);
    glActiveTexture(GL_TEXTURE0);
	glUniform1i(shaders[currShader].TextureID, 0);
    glBindTexture(GL_TEXTURE_2D, map->Texture);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(shaders[currShader].VertexID);
	glBindBuffer(GL_ARRAY_BUFFER, map->vertexbuffer);
	glVertexAttribPointer(
		shaders[currShader].VertexID,	// attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(shaders[currShader].UVID);
    glBindBuffer(GL_ARRAY_BUFFER, map->uvbuffer);
    glVertexAttribPointer(
        shaders[currShader].UVID,                   // The attribute we want to configure
        2,                            // size : U+V => 2
        GL_FLOAT,                     // type
        GL_FALSE,                     // normalized?
        0,                            // stride
        (void*)0                      // array buffer offset
    );

	// Draw the\ triangle !
	//glDrawArrays(GL_TRIANGLES, 0, map->numIndices); // 3 indices starting at 0 -> 1 triangle
///*
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, map->elementbuffer);
    // Draw the triangles !
    glDrawElements(
        GL_TRIANGLES,      // mode
        map->numIndices,        // count
        GL_UNSIGNED_SHORT, // type
        (void*)0           // element array buffer offset
    );
	//*/
	glDisableVertexAttribArray(shaders[currShader].UVID);
	glDisableVertexAttribArray(shaders[currShader].VertexID);
}

void MAP_Init() {
//	programID = RE_LoadShaders("simple.vert", "simple.frag");
	MAP_LoadShaders();
}

void MAP_Cleanup() {
    //glDeleteProgram(programID);
	MAP_FreeShaders();
}
