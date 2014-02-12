#version 120

// Interpolated values from the vertex shaders
invarying vec2 UV;
invarying vec3 LightDiffuseColor;
invarying vec3 LightSpecularColor;

// Values that stay constant for the whole mesh.
uniform sampler2D tex;

void main(){
	vec3 MaterialDiffuseColor = texture2D( tex, UV ).rgb;
	vec3 MaterialAmbientColor = vec3(0.2,0.2,0.2) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

	gl_FragColor = 
		vec4( MaterialAmbientColor +
		MaterialDiffuseColor * LightDiffuseColor +
		MaterialSpecularColor * LightSpecularColor, 1 );

}