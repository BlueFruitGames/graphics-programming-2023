#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec4 Color;
uniform sampler2D ColorTexture;
uniform vec3 AmbientColor;
uniform float AmbientReflection;

vec3 GetAmbientReflection(){
	return texture(ColorTexture, TexCoord).xyz * AmbientColor * AmbientReflection;
}

vec3 GetBlinnPhongReflection() {
	return GetAmbientReflection();
}


void main()
{
	FragColor = Color * vec4(GetBlinnPhongReflection(),1);
}

