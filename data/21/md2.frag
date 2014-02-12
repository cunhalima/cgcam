#version 120

invarying vec2 UV;

uniform sampler2D tex;

void main() {
    gl_FragColor = texture2D(tex, UV);
}