#version 120

// Input vertex data, different for all executions of this shader.
in vec3 vertex_ms;
in vec2 uv_ts;

// Output data ; will be interpolated for each fragment.
outvarying vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 light_ws;

void main() {
	gl_Position = MVP * vec4(vertex_ms, 1);

	// Output position of the vertex, in clip space : MVP * position
	//gl_Position.xyz = vertex_ms.xyz;
	//gl_Position.w = 1.0f;

	// UV of the vertex. No special space for this one.
	UV = uv_ts;
}
