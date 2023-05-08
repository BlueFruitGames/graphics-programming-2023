// Uniforms
// Replace constants with uniforms with the same name

#define PI 3.1415926538

//Bend Uniforms
uniform float BendStrength = 0.0f;
uniform float BendDistanceFactor = 0.0f;
uniform float BendStartOffset = 0.0f;

//Ground Uniforms
uniform float GroundOffset = -5.0f;
uniform vec3 GroundNormal = vec3(0, 1, 0);
uniform vec3 GroundColor = vec3(1,1,1);
uniform float GroundSpeed = 3.0f;

//BlackHole Uniforms
uniform vec3 BlackHoleColor = vec3(0, 0, 1);
uniform vec3 BlackHoleStartPosition = vec3(-2, 0, -10);
uniform float BlackHoleRadius = 1.25f;
uniform vec2 BlackHoleInfluenceBounds = vec2(-1,1);
uniform float BlackHoleInfluence =5.0f;
uniform float FresnelPower = 1.0f;
uniform float FresnelStrength = 1.0f;
uniform vec3 FresnelColor = vec3(0, 0, 1);

//BlackHoleParticles Uniforms
uniform float BlackHoleParticlesPullSpeed = 1.0f;
uniform float BlackHoleParticlesRotationSpeed = 1.0f;
uniform int BlackHoleParticlesAmount = 1;
uniform float BlackHoleParticlesSpawnDistance = 10.0f;
uniform float BlackHoleParticlesRadius = 1.0f;
uniform int BlackHoleParticlesLayers = 1;
uniform float BlackHoleParticlesSmoothness = 1;
uniform float BlackHoleParticlesRotationOffset = 0.0f;

//General Uniforms
uniform float Time = 0.0f;
uniform float Smoothness = 1.0f;
uniform mat4 ViewMatrix;

//Textures
uniform sampler2D GroundTexture;
uniform vec2 GroundTextureScale;

uniform sampler2D BlackHoleTexture;
uniform vec2 BlackHoleTextureScale;

uniform sampler2D BlackHoleParticlesTexture;
uniform vec2 BlackHoleParticlesTextureScale;

uniform sampler2D BackgroundTexture;
uniform vec2 BackgroundTextureScale;

// Output structure
struct Output
{
	// color of the closest figure
	vec3 color;
	vec3 groundColor;
	
	float groundWeight;
	
	float blackHoleImpact;
	float blackHoleBlending;
};

struct BlackHoleInfo
{
	vec3 position;
	vec2 influenceBounds;
	float influence;
	float radius;
};


float pcurve( float x, float a, float b ){
	float k = pow(a+b,a+b) / (pow(a,a)*pow(b,b));
	return k * pow( x, a ) * pow( 1.0-x, b );
}

float createBlackholeParticles(vec3 p, vec3 spherePosition){
	float d = 0;
	float baseDeltaAngle = 2 * PI / BlackHoleParticlesAmount;
	int maxEdges = 5;
	for (int edgeCount = 0; edgeCount < BlackHoleParticlesLayers; ++edgeCount){
		float deltaAngle = baseDeltaAngle + BlackHoleParticlesRotationOffset * edgeCount;
		for(int i = 0; i < BlackHoleParticlesAmount; ++i)
		{
			float Offset = float(edgeCount)/float(BlackHoleParticlesLayers);
			//sine -> sawtooth
			float currentPullTime = (1 - sin(Offset + Time * BlackHoleParticlesPullSpeed - floor(Offset + Time * BlackHoleParticlesPullSpeed)));
	
			vec3 currentParticle = normalize(vec3(
			sin(deltaAngle * float(i) + Time * BlackHoleParticlesRotationSpeed + Offset),
			0,
			cos(deltaAngle* float(i) + Time * BlackHoleParticlesRotationSpeed + Offset)
			));
			currentParticle *= currentPullTime;
	
	
			float distanceToSphere = currentPullTime * BlackHoleParticlesSpawnDistance;
			float normalizedDistance = smoothstep(0, BlackHoleParticlesSpawnDistance-BlackHoleRadius, distanceToSphere);
			//parabola shaping function 
			float currentRadius = pcurve(normalizedDistance, .6, 3) * BlackHoleParticlesRadius;
			vec3 particlePos = TransformToLocalPoint(p, spherePosition) + currentParticle * BlackHoleParticlesSpawnDistance;
			float displacement = sin(.5* p.x) * sin(.5 * p.y) * sin(.75 * p.z) * .5;
			float dSphere = SphereSDF(particlePos, currentRadius) + displacement;
			if(i == 0 && edgeCount ==0)
				d = dSphere;
			else
				d = SmoothUnion(d, dSphere, BlackHoleParticlesSmoothness);
		}
	}
	return d;
}

float createBlackHole(vec3 p, vec3 spherePosition){

	float dBlackhole = SphereSDF(TransformToLocalPoint(p, spherePosition), BlackHoleRadius);
	return dBlackhole;
}

float createGround(vec3 p, BlackHoleInfo blackHoleInfo, out float blackHoleImpact){
	float dGroundPlane = BendedPlaneSDF(p, GroundNormal, blackHoleInfo, Time * GroundSpeed, blackHoleImpact);
	return dGroundPlane - GroundOffset;
}

float createHouse(){
	return 0;
}

float setupColor(){
	return 0;
}

// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
	//Setup of BlackholeInfo struct
	float currentZ = clamp(0, p.z, p.z - BendStartOffset); 
	float BendOffsetY = pow(currentZ * BendDistanceFactor, 2) * -BendStrength;
	vec3 blackHolePosition = BlackHoleStartPosition;
	blackHolePosition.y -= BendOffsetY;
	BlackHoleInfo blackHoleInfo;
	blackHoleInfo.influence = BlackHoleInfluence;
	blackHoleInfo.position = blackHolePosition;
	blackHoleInfo.influenceBounds = BlackHoleInfluenceBounds;
	blackHoleInfo.radius = BlackHoleRadius;

	//Creation of objects
	vec3 groundInfluencedColor;
	float dGroundPlane = createGround(p, blackHoleInfo, o.blackHoleImpact) + BendOffsetY;
	float dBlackhole = createBlackHole(p, blackHolePosition);
	float dBlackholeParticles = createBlackholeParticles(p, blackHolePosition);
	
	
	//Blending of colors
	float d = SmoothUnion(dBlackhole, dBlackholeParticles, Smoothness, o.blackHoleBlending);
	
	d = Union(d, dGroundPlane);
	//pass the result of the condition or weight for material
	o.groundWeight = (d == dGroundPlane) ? 1.0f : 0.0f;
	return d;
}



// Default value for o
void InitOutput(out Output o)
{
	o.color = vec3(0.0f);
}

// Output function: Just a dot with the normal and view vectors
vec4 GetOutputColor(vec3 p, float distance, vec3 dir, Output o)
{
	vec3 normal = CalculateNormal(p);
	vec3 groundPlaneTexColor = texture(GroundTexture, p.xz * GroundTextureScale).rgb;
	vec3 groundColor = mix(groundPlaneTexColor, vec3(1,0,0), o.blackHoleImpact);

	
	vec3 triPlanerNormal = abs(normal);
	triPlanerNormal *= pow(triPlanerNormal, vec3(5));
	triPlanerNormal /= triPlanerNormal.x + triPlanerNormal.y + triPlanerNormal.z;
	triPlanerNormal = (p) / (p.x + p.y + p.z);

	mat3x3 triKrn = mat3x3(texture(BlackHoleTexture, abs(p.yz)).rgb,
	texture(BlackHoleTexture, abs(p.xz)).rgb,
	texture(BlackHoleTexture, abs(p.xy)).rgb);
	
	vec3 blackHoleColorXY = texture(BlackHoleTexture,normal.xy * BlackHoleTextureScale).rgb;
	vec3 blackHoleColorXZ = texture(BlackHoleTexture,normal.xz * BlackHoleTextureScale).rgb;
	vec3 blackHoleColorYZ = texture(BlackHoleTexture,normal.yz * BlackHoleTextureScale).rgb;
	
	vec3 blackHoleColorFinal = blackHoleColorXY;
	//blackHoleColorFinal = (triKrn * abs(normal));

	float fresnelFactor = dot(normal, dir); 
	fresnelFactor = max(0, 1 - fresnelFactor);
	fresnelFactor = pow(fresnelFactor, FresnelPower);
	
	vec3 blackHoleColor = mix(mix(blackHoleColorFinal, FresnelColor * FresnelStrength, fresnelFactor), vec3(0.4), o.blackHoleBlending);
	o.color  = mix(blackHoleColor, groundColor, o.groundWeight);
	
	vec3 viewDir = normalize(-p);
	float dotNV = dot(normalize(-p), normal);
	return vec4(dotNV * o.color, 1.0f);
}