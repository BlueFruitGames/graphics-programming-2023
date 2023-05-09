//Inputs
in vec2 TexCoord;

//Outputs
out vec4 FragColor;

//Uniforms
uniform mat4 ProjMatrix;
uniform mat4 InvProjMatrix;
uniform mat4 ViewMatrix;
uniform samplerCube CubeMap;



// Implement GetDistance based on version with output
float GetDistance(vec3 p)
{
	Output o;
	return GetDistance(p, o);
}

// Configure ray marcher
void GetRayMarcherConfig(out uint maxSteps, out float maxDistance, out float surfaceDistance)
{
    maxSteps = 100u;
    maxDistance = ProjMatrix[3][2] / (ProjMatrix[2][2] + 1.0f); // Far plane
    surfaceDistance = 0.001f;
}

void main()
{
	// Start from transformed position
	vec4 viewPos = InvProjMatrix * vec4(TexCoord.xy * 2.0f - 1.0f, 0.0f, 1.0f);
	vec3 origin = viewPos.xyz / viewPos.w;

	// Initial distance to camera
	float distance = length(origin);

	// Normalize to get view direction
	vec3 dir = origin / distance;

	// Get Distance from the origin to the closest object
	distance += RayMarch(origin, dir);

	// Hit point in view space is given by the direction from the camera and the distance
	vec3 point = dir * distance;

	// Invoke GetDistance again to get the output value
	Output o;
	InitOutput(o);
	GetDistance(point, o);
	
	uint maxSteps;
	float maxDistance, surfaceDistance;
	GetRayMarcherConfig(maxSteps, maxDistance, surfaceDistance);

	mat4 cubeMapViewMatrix;
	cubeMapViewMatrix = ViewMatrix;
	cubeMapViewMatrix[3][0] = 0;
	cubeMapViewMatrix[3][1] = 0;
	cubeMapViewMatrix[3][2] = 0;
	// With the output value, get the final color
	FragColor = distance < maxDistance? GetOutputColor(point, distance, dir, o) : vec4(texture(CubeMap, (cubeMapViewMatrix * viewPos).rgb).rgb,1);

	// Convert linear depth to normalized depth (same as projecting the point and taking the Z/W)
	//gl_FragDepth = -ProjMatrix[2][2] - ProjMatrix[3][2] / point.z;
}
