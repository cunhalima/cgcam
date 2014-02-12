#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include <cassert>
#include <vector>
//#include <cstring>
#include "../defs.h"
//

static GLuint programID;
static GLuint vertexbuffer;
static GLuint VertexID;
static GLuint MVPID;

void RE_AddSplash(vec3_t origin) {
	refParticle_t p;

	p.deathtime = g_Time + 1000;
	M_VectorCopy(p.origin, origin);
	p.type = 0;
	M_VectorCopy(p.velocity, origin);
	RE_AddParticleToScene(&p);
}

// should have a function to add a "refresh entity" to the scene
void RE_AddParticleToScene(refParticle_t *original) {
	assert(original != NULL);
	for (int i = 0; i < MAX_DRAW_PARTICLES - 1; i++) {
		refParticle_t *p = &rs.particles[i];
		if (p->type < 0) {
			*p = *original;
			return;
		}
	}
}

void RE_RenderParticles(const float *matV, const float *matP) {
	glm::vec3 pos = glm::vec3(3, 3, 3);


	glm::mat4 &V  = *((glm::mat4 *)matV);
	glm::mat4 &P  = *((glm::mat4 *)matP);
	glm::mat4 M = glm::mat4(1);

	glm::vec3 right = glm::vec3(V[0][0], V[1][0], V[2][0]);
	glm::vec3 up = glm::vec3(V[0][1], V[1][1], V[2][1]);

	//M = glm::translate(M, glm::vec3(pos.x, pos.y, pos.z));

	glm::mat4 MVP = P * V * M;
	std::vector<glm::vec3> vpos;

	vpos.push_back(pos);
	vpos.push_back(pos + right);
	vpos.push_back(pos + up);
	vpos.push_back(pos + up);
	vpos.push_back(pos + right);
	vpos.push_back(pos + right + up);

	for (int i = 0; i < MAX_DRAW_PARTICLES - 1; i++) {
		refParticle_t *p = &rs.particles[i];
		if (p->type < 0) {
			continue;
		}
		if (g_Time >= p->deathtime) {
			p->type = -1;
			continue;
		}
		pos.x = p->origin[0];
		pos.y = p->origin[1];
		pos.z = p->origin[2];
		float psize = 0.5f;

		vpos.push_back(pos - right * psize - up * psize);
		vpos.push_back(pos + right * psize - up * psize);
		vpos.push_back(pos - right * psize + up * psize);
		vpos.push_back(pos - right * psize + up * psize);
		vpos.push_back(pos + right * psize - up * psize);
		vpos.push_back(pos + right * psize + up * psize);
	}


	glUseProgram(programID);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vpos.size()*sizeof(vpos[0]), &vpos[0], GL_STATIC_DRAW);


	// Use our shader
	//con_printf("%d\n", programID);

	glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(VertexID);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		VertexID,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the\ triangle !
	glDrawArrays(GL_TRIANGLES, 0, (int)vpos.size()); // 3 indices starting at 0 -> 1 triangle
	glDisableVertexAttribArray(VertexID);
}

void RE_ParticlesInit() {
	programID = RE_LoadShaders( "billboard.vert", "billboard.frag" );
	VertexID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	MVPID = glGetUniformLocation(programID, "MVP");

	static const GLfloat g_vertex_buffer_data[] = { 
		-1.0f, 0.0f, -1.0f,
		 1.0f, 0.0f, -1.0f,
		 0.0f, 0.0f, 1.0f,
	};

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
}

void RE_ParticlesCleanup() {
	glDeleteBuffers(1, &vertexbuffer);
}
