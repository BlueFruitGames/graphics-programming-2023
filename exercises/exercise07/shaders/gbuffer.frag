//Inputs
in vec3 ViewNormal;
in vec2 TexCoord;

//Outputs
out vec4 FragAlbedo;
out vec2 FragNormal;
out vec4 FragOthers;

//Uniforms
uniform vec3 Color;
uniform sampler2D ColorTexture;

uniform float AmbientReflectance;
uniform float DiffuseReflectance;
uniform float SpecularReflectance;
uniform float SpecularExponent;

void main()
{
	FragAlbedo = vec4(texture(ColorTexture, TexCoord).rgb * Color, 1.0f); 
	FragNormal = ViewNormal.xy;
	FragOthers = vec4(SpecularReflectance, SpecularReflectance, SpecularReflectance, 1 / (1 + SpecularExponent));
}
