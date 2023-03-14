#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;

uniform sampler2D ColorTexture;
uniform vec3 AmbientColor;
uniform float AmbientReflection;

uniform float DiffuseReflection;
uniform vec3 LightColor;
uniform vec3 LightPosition;


vec3 GetAmbientReflection(){

	return AmbientColor * AmbientReflection * texture(ColorTexture, TexCoord).xyz;
}

vec3 GetDiffuseReflection(vec3 LightVector, vec3 WorldNormalNormalized){
	return texture(ColorTexture, TexCoord).xyz * LightColor * DiffuseReflection * clamp(dot(WorldNormalNormalized,LightVector),0,1);
}

vec3 GetBlinnPhongReflection(vec3 LightVector, vec3 WorldNormalNormalized) {
	return GetAmbientReflection() + GetDiffuseReflection(LightVector, WorldNormalNormalized);
}


void main()
{
	vec3 LightVector = LightPosition - WorldPosition;
	vec3 LightVectorNormalized = normalize(LightVector);
	vec3 WorldNormalNormalized = normalize(WorldNormal);
	FragColor = Color * vec4(GetBlinnPhongReflection(LightVectorNormalized,WorldNormalNormalized),1);
}

