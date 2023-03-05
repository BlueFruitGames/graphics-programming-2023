#version 330 core

in vec3 WorldPosition;
in vec3 WorldNormal;
in vec2 TexCoord;
in float Height;

uniform sampler2D DirtTexture;
uniform sampler2D GrassTexture;
uniform sampler2D RockTexture;
uniform sampler2D SnowTexture;

uniform vec2 GrassHeightRange;
uniform vec2 RockHeightRange;
uniform vec2 SnowHeightRange;


uniform vec2 ColorTextureScale;

out vec4 FragColor;

float mapRange(vec2 range, float height){

 return clamp((height - range.x) / (range.y - range.x), 0, 1);
}

void main()
{
	vec4 DirtColor = texture(DirtTexture, TexCoord * ColorTextureScale);
	vec4 GrassColor = texture(GrassTexture, TexCoord * ColorTextureScale);
	vec4 RockColor = texture(RockTexture, TexCoord * ColorTextureScale);
	vec4 SnowColor = texture(SnowTexture, TexCoord * ColorTextureScale);

	float alphaDirtGrass = mapRange(GrassHeightRange, Height);
	float alphaGrassRock = mapRange(RockHeightRange, Height);
	float alphaRockSnow = mapRange(SnowHeightRange, Height);
	vec4 FinalColor = DirtColor;
	FinalColor = mix(FinalColor, GrassColor, alphaDirtGrass);
	FinalColor = mix(FinalColor, RockColor, alphaGrassRock);
	FinalColor = mix(FinalColor, SnowColor, alphaRockSnow);

	FragColor = FinalColor;
}
