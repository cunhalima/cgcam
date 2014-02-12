#version 120

// Input vertex data, different for all executions of this shader.
attribute vec2 xy_in;
attribute vec2 uv_in;
attribute vec3 rgb_in;

// Output data ; will be interpolated for each fragment.
outvarying vec2 UV;
outvarying vec3 RGB;

void main()
{
    gl_Position = vec4(xy_in, 0, 1);
    UV = uv_in;
    RGB = rgb_in;
}

