#version 330 core

layout (location = 0) in vec2 ParticlePosition;
// (todo) 02.X: Add more vertex attributes
layout (location = 1) in float size;
layout (location = 2) in float birth;
layout (location = 3) in float duration;

// (todo) 02.5: Add Color output variable here


// (todo) 02.X: Add uniforms
uniform float CurrentTime;

void main()
{
	gl_Position = vec4(ParticlePosition, 0.0, 1.0);
	if(CurrentTime - birth > duration) 
		gl_PointSize = 0;
	else
		gl_PointSize = size;
}
