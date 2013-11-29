#version 120

// Interpolated values from the vertex shaders
varying vec3 fragmentColor;
varying vec2 UV;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){
    // Output color = color of the texture at the specified UV
    gl_FragColor = texture2D( myTextureSampler, UV ) * vec4(fragmentColor, 1);
}
