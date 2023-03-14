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

uniform float SpecularReflection;
uniform float SpecularExponent;
uniform vec3 CameraPosition;


vec3 GetAmbientReflection(){

	return AmbientColor * AmbientReflection * texture(ColorTexture, TexCoord).xyz;
}

vec3 GetDiffuseReflection(vec3 LightVector, vec3 NormalVector){
	return LightColor * DiffuseReflection * texture(ColorTexture, TexCoord).xyz * max(dot(LightVector,NormalVector),0);
}

vec3 GetSpecularReflection(vec3 NormalVector, vec3 HalfVector){
	return LightColor * SpecularReflection * pow(max(dot(NormalVector, HalfVector), 0.0f), SpecularExponent);
}

vec3 GetBlinnPhongReflection(vec3 LightVector, vec3 NormalVector, vec3 HalfVector) {
	return GetAmbientReflection() + GetDiffuseReflection(LightVector, NormalVector) + GetSpecularReflection(NormalVector, HalfVector);
}


void main()
{
	vec3 LightVector = normalize(LightPosition - WorldPosition);
	vec3 NormalVector = normalize(WorldNormal);
	vec3 ViewVector = normalize(CameraPosition - WorldPosition);
	vec3 HalfVector = normalize(LightVector + ViewVector);
	FragColor = Color * vec4(GetBlinnPhongReflection(LightVector,NormalVector, HalfVector),1);
}

