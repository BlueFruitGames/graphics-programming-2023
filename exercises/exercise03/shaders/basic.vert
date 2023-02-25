#version 330 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

// (todo) 03.X: Add the out variables here
out vec3 Normal;
out vec3 Position;

// (todo) 03.X: Add the uniforms here
uniform mat4 WorldMatrix = mat4(1.0f);
uniform mat4 RotationMatrix = mat4(1.0f);
uniform mat4 ViewProjMatrix = mat4(1.0f);

void main()
{
	gl_Position = ViewProjMatrix * WorldMatrix * RotationMatrix * vec4(VertexPosition, 1.0);
	// Objectspace Position -> Position = VertexPosition;
	// Worldspace Position -> Position = vec3(WorldMatrix * vec4(VertexPosition,0));
	//Clipdspace Position
	Position = vec3(ViewProjMatrix * WorldMatrix * vec4(VertexPosition,0));
	Normal = vec3(WorldMatrix * vec4(VertexNormal,0));
}
