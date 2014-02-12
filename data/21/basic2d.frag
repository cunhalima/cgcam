#version 120

invarying vec3 RGB;

void main() {
    gl_FragColor = vec4(RGB, 1);
}
