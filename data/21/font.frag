#version 120

invarying vec2 UV;
invarying vec3 RGB;
uniform sampler2D tex;

void main()
{
    gl_FragColor = texture2D(tex, UV) * vec4(RGB, 1);
}
