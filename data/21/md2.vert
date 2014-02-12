#version 120

in vec3 pos_from;
in vec3 pos_to;
in vec2 uv;

// Output data ; will be interpolated for each fragment.
outvarying vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform float amount;

void main() {
    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  MVP * vec4(mix(pos_from, pos_to, amount), 1);
    //fragmentColor = vertexColor;
    // UV of the vertex. No special space for this one.
    UV = uv;
}