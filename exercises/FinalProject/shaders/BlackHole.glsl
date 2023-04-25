
// Uniforms
// Replace constants with uniforms with the same name

uniform float PlaneOffset = -5.0f;
uniform vec3 PlaneNormal = vec3(0, 1, 0);
uniform vec3 PlaneColor = vec3(1,1,1);
uniform vec2 BendDistanceBounds = vec2(-1,1);
uniform vec3 BendOrigin = vec3(0, 0, 0);
uniform float AnimationSpeed = 3.0f;

uniform vec3 SphereColor = vec3(0, 0, 1);
uniform vec3 SphereStartPosition = vec3(-2, 0, -10);
uniform float SphereRadius = 1.25f;
//uniform float SphereRotationSpeed = 1f;

uniform vec2 SphereInfluenceBounds = vec2(-1,1);
uniform float SphereInfluence =5.0f;

uniform float BlackHoleParticlesPullSpeed = 1.0f;
uniform int BlackHoleParticlesAmount = 1;
uniform float BlackHoleParticlesSpawnDistance = 10.0f;
uniform float BlackHoleParticlesRadius = 1.0f;

uniform float Time = 0.0f;

#define PI 3.1415926538

// Output structure
struct Output
{
	// color of the closest figure
	vec3 color;
};

float createBlackholeParticles(vec3 p, vec3 spherePosition){
	float d = 0;
	float deltaAngle = 2 * PI / BlackHoleParticlesAmount;
	for(int i = 0; i < BlackHoleParticlesAmount; ++i)
	{
		vec3 currentParticle = normalize(vec3(
			sin(deltaAngle * float(i) + Time * BlackHoleParticlesPullSpeed), 
			0, 
			cos(deltaAngle* float(i) + Time * BlackHoleParticlesPullSpeed)
		));
		float dSphere = SphereSDF(TransformToLocalPoint(p, spherePosition) + currentParticle * 10.0f, 
		BlackHoleParticlesRadius);
		if(i == 0)
			d = dSphere;
		else
			d = Union(d, dSphere);
	}
	return d;
}

// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
	// Sphere in position "SphereCenter" and size "SphereRadius"
	//float dSphere = SphereSDF(TransformToLocalPoint(p, SphereCenter), SphereRadius);

	// Box with worldView transform "BoxMatrix" and dimensions "BoxSize"
	//float dBox = BoxSDF(TransformToLocalPoint(p, BoxMatrix), BoxSize);

	// Replace Union with SmoothUnion and try different small values of smoothness
	//float blend;
	//float d = SmoothUnion(dSphere, dBox, Smoothness, blend);

	// Replace this with a mix, using the blend factor from SmoothUnion

	//vec3 spherePosition = vec3(BendOrigin.x + cos(Time * SphereRotationSpeed) * SphereStartPosition.z,
	//SphereStartPosition.y + sin(Time * SphereRotationSpeed * 2) * 8.0f,
	//BendOrigin.z - sin(Time * SphereRotationSpeed) * SphereStartPosition.z);

	vec3 spherePosition = SphereStartPosition + BendOrigin;
	
	float sphereImpact;
	
	float dGroundPlane = BendedPlaneSDF(p, PlaneNormal, PlaneOffset, BendOrigin, BendDistanceBounds, 
	spherePosition, SphereInfluenceBounds, SphereInfluence, SphereRadius, Time * AnimationSpeed, sphereImpact);

	float dSphere = SphereSDF(TransformToLocalPoint(p, spherePosition), SphereRadius);
	float dBlackholeParticles = createBlackholeParticles(p, spherePosition); 
	
	
	//o.color = mix(SphereColor, BoxColor, blend);
	//d = dGroundPlane;
	float d = Union(Union(dSphere, dGroundPlane), dBlackholeParticles);
	float dTest = dBlackholeParticles;

	vec3 baseColor = mix(PlaneColor, vec3(1,0,0), sphereImpact);

	o.color  = d == dSphere ? SphereColor : baseColor;



	return dTest;
}



// Default value for o
void InitOutput(out Output o)
{
	o.color = vec3(0.0f);
}

// Output function: Just a dot with the normal and view vectors
vec4 GetOutputColor(vec3 p, float distance, Output o)
{
	vec3 normal = CalculateNormal(p);
	vec3 viewDir = normalize(-p);
	float dotNV = dot(normalize(-p), normal);
	return vec4(dotNV * o.color, 1.0f);
}
