#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>
#include <cstring>
#include "../defs.h"
//

// should have a function to add a "refresh entity" to the scene
void RE_AddRefEntityToScene(refEntity_t *original) {
	assert(original != NULL);
	if (rs.numDrawEntities >= MAX_DRAW_ENTITIES) {
		return;
	}
	if (original->model < 0 || original->model >= MAX_MODELS) {
		return;
	}

	refEntity_t *e = &rs.drawEnts[rs.numDrawEntities++];
	*e = *original;
	e->next = rs.models[e->model];
	rs.models[e->model] = e;
}

void RE_BeginScene() {
	rs.numDrawEntities = 0;
	memset(rs.models, 0, sizeof(rs.models));


}

void RE_EndScene() {

}

void GetViewMatrix(glm::mat4& V) {
	/*
	glm::mat4 V = glm::lookAt(glm::vec3(0, -8, 2),
								glm::vec3(0, 0, 0),
								glm::vec3(0, 0, 1));
	*/
//#define 
	float pi = (float)acos(-1.0);

	//float yaw_rad = cs.viewAngles[ANG_YAW] * pi / 180.0f;
	//float pitch_rad = cs.viewAngles[ANG_PITCH] * pi / 180.0f;

	V = glm::mat4(1);


	V = glm::rotate(V, 90.0f, glm::vec3(0, 1, 0));
	V = glm::rotate(V, 90.0f, glm::vec3(-1, 0, 0));

	V = glm::rotate(V, -cs.viewAngles[ANG_ROLL], glm::vec3(1, 0, 0));
	V = glm::rotate(V, -cs.viewAngles[ANG_PITCH], glm::vec3(0, 1, 0));
	V = glm::rotate(V, -cs.viewAngles[ANG_YAW], glm::vec3(0, 0, 1));


	V = glm::translate(V, glm::vec3(-cs.viewOrigin[0], -cs.viewOrigin[1], -cs.viewOrigin[2]));

	/*

	float yaw_sin = sin(yaw_rad);
	float yaw_cos = cos(yaw_rad);




	V = glm::lookAt(glm::vec3(cs.viewOrigin[0], cs.viewOrigin[1], cs.viewOrigin[2]),
					glm::vec3(cs.viewOrigin[0] + yaw_cos, cs.viewOrigin[1] + yaw_sin, cs.viewOrigin[2]),
					glm::vec3(0, 0, 1));
	*/
}


void RE_RenderScene(float lerp) {
	glm::mat4 P = glm::perspective(45.0f, float(rs.xres) / float(rs.yres), 0.1f, 100.0f);
	glm::mat4 V;
	GetViewMatrix(V);




	MDL_BeginRender(1, lerp);
	MDL_SetViewMatrix(&V[0][0]);
	glm::vec3 lpos = glm::vec3(8 * 8, 8 * 8, 4);
	MDL_SetLightPos(&lpos.x);
	MDL_SetLightParams(0.05f, 0.5f);
	for (int i = 0; i < MAX_MODELS; i++) {
		refEntity_t *e = rs.models[i];
		if (e == NULL ) {
			continue;
		}
		MDL_t *mdl = MDL_Get(e->model);
		if (mdl == NULL) {
			continue;
		}

		//MDL_PrepareTexture(e->model);
		MDL_PrepareTexture(mdl);

		for ( ; e != NULL; e = e->next) {
			int model = e->model;
			glm::mat4 M = glm::mat4(1);
			M = glm::translate(M, glm::vec3(e->origin[0], e->origin[1], e->origin[2]));
			M = glm::scale(M, glm::vec3(0.1f));
			//M = glm::rotate(M, e->angles[ANG_PITCH], glm::vec3(0, 1, 0));
			M = glm::rotate(M, e->angles[ANG_YAW], glm::vec3(0, 0, 1));
			//M = glm::rotate(M, e->angles[ANG_ROLL], glm::vec3(1, 0, 0));
			glm::mat4 MVP = P * V * M;
			MDL_SetModelMatrix(&M[0][0]);
			MDL_SetMVPMatrix(&MVP[0][0]);
			MDL_Render(mdl, e->frame1, e->frame2);
		}
	}
	MDL_EndRender();
	if (thismap != NULL) {
		glm::mat4 M = glm::mat4(1);
		glm::mat4 MVP = P * V * M;
		MAP_BeginRender();
		MAP_SetModelMatrix(&M[0][0]);
		MAP_SetViewMatrix(&V[0][0]);
		MAP_SetMVPMatrix(&MVP[0][0]);
		MAP_Render(thismap);
	}

	// z  * 2 - 1

	// 0 -> -1
	// 1 -> 1

#if 1
	float centerZ = RE_GetCenterZ();
	glm::mat4 VP = P * V;
	glm::mat4 invVP = glm::inverse(VP);
	glm::vec4 res = invVP * glm::vec4(0.0f, 0.0f, centerZ * 2.0f - 1.0f, 1.0f);
	res /= res.w;
	rs.targetOrigin[0] = res[0];
	rs.targetOrigin[1] = res[1];
	rs.targetOrigin[2] = res[2];
#endif
	//con_printf("center at %f %f %f\n", center[0], center[1], center[2]);

	RE_RenderParticles(&V[0][0], &P[0][0]);

}