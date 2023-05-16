// Uniforms
// Replace constants with uniforms with the same name

#define PI 3.1415926538


//General Uniforms
uniform int isWorldBendingActive = 0;
uniform int isGroundAnimationActive = 0;
uniform int isFresnelActive = 1;
uniform vec3 FresnelColor = vec3(0, 0, 1);

uniform float Time = 0.0f;
uniform float Smoothness = 1.0f;

//SkyBox uniforms
uniform sampler2D BackgroundTexture;
uniform vec2 BackgroundTextureScale;

//Bend Uniforms
uniform float BendStrength = 0.0f;
uniform float BendDistanceFactor = 0.0f;
uniform float BendStartOffset = 0.0f;

//Ground Uniforms
uniform sampler2D GroundTexture;
uniform vec2 GroundTextureScale;
uniform vec3 GroundNormal = vec3(0, 1, 0);
uniform float GroundOffset = -5.0f;
uniform float GroundSpeed = 3.0f;


//BlackHole Uniforms
uniform int isBlackHoleTextureMoving = 1;
uniform sampler2D BlackHoleTexture;
uniform vec2 BlackHoleTextureScale;
uniform vec3 BlackHoleColor = vec3(0, 0, 1);
uniform vec3 BlackHolePosition = vec3(-2, 0, -10);
uniform float BlackHoleRadius = 1.25f;
uniform vec2 BlackHoleInfluenceBounds = vec2(-1,1);
uniform float BlackHoleInfluence =5.0f;
uniform float FresnelPower = 1.0f;
uniform float FresnelStrength = 1.0f;

//BlackHoleParticles Uniforms
uniform sampler2D BlackHoleParticlesTexture;
uniform vec2 BlackHoleParticlesTextureScale;
uniform float BlackHoleParticlesPullSpeed = 1.0f;
uniform float BlackHoleParticlesRotationSpeed = 1.0f;
uniform float BlackHoleParticlesSpawnDistance = 10.0f;
uniform float BlackHoleParticlesRadius = 1.0f;
uniform int BlackHoleParticlesColumnCount = 1;
uniform int BlackHoleParticlesPerColumn = 1;
uniform float BlackHoleParticlesSmoothness = 1;
uniform float BlackHoleParticlesRotationOffset = 0.0f;


// Forward declare config function
void GetRayMarcherConfig(out uint maxSteps, out float maxDistance, out float surfaceDistance);

// Output structure
struct Output
{
	// color of the closest figure
	vec3 color;
	vec3 groundColor;
	
	float groundWeight;
	float backgroundWeight;
	
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
	float baseDeltaAngle = 2 * PI / BlackHoleParticlesColumnCount;
	int maxEdges = 5;
	for (int x = 0; x < BlackHoleParticlesPerColumn; ++x){
		float deltaAngle = baseDeltaAngle + BlackHoleParticlesRotationOffset * x;
		for(int y = 0; y < BlackHoleParticlesColumnCount; ++y)
		{
			float Offset = float(x)/float(BlackHoleParticlesPerColumn);
			//sine -> sawtooth
			float currentPullTime = (1 - sin(Offset + Time * BlackHoleParticlesPullSpeed - floor(Offset + Time * BlackHoleParticlesPullSpeed)));
	
			vec3 currentParticle = normalize(vec3(
			sin(deltaAngle * float(y) + Time * BlackHoleParticlesRotationSpeed + Offset),
			0,
			cos(deltaAngle* float(y) + Time * BlackHoleParticlesRotationSpeed + Offset)
			));
			currentParticle *= currentPullTime;
	
	
			float distanceToSphere = currentPullTime * BlackHoleParticlesSpawnDistance;
			float normalizedDistance = smoothstep(0, BlackHoleParticlesSpawnDistance, distanceToSphere);
			//parabola shaping function 
			float currentRadius = pcurve(normalizedDistance, .6, 3) * BlackHoleParticlesRadius;
			vec3 particlePos = TransformToLocalPoint(p, spherePosition) + currentParticle * BlackHoleParticlesSpawnDistance;
			float dSphere = SphereSDF(particlePos, currentRadius);
			if(y == 0 && x ==0)
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
	float dGroundPlane = BendedPlaneSDF(p, GroundNormal, blackHoleInfo, Time * GroundSpeed * isGroundAnimationActive, blackHoleImpact);
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
	float currentZ = max(0, abs(p.z) - BendStartOffset); 
	float BendOffsetY = pow(currentZ * BendDistanceFactor, 2) * -BendStrength * isWorldBendingActive;
	vec3 blackHolePosition = BlackHolePosition;
	blackHolePosition.y -= BendOffsetY;
	BlackHoleInfo blackHoleInfo;
	blackHoleInfo.influence = BlackHoleInfluence;
	blackHoleInfo.position = blackHolePosition;
	blackHoleInfo.influenceBounds = BlackHoleInfluenceBounds;
	blackHoleInfo.radius = BlackHoleRadius;

	
	//Creation of objects
	float dGroundPlane = createGround(p, blackHoleInfo, o.blackHoleImpact) + BendOffsetY;
	float dBlackhole = createBlackHole(p, blackHolePosition);
	float dBlackholeParticles = createBlackholeParticles(p, blackHolePosition);
	
	
	//Blending of colors
	float d = SmoothUnion(dBlackhole, dBlackholeParticles, Smoothness, o.blackHoleBlending);
	
	d = Union(d, dGroundPlane);
	//pass the result of the condition or weight for material
	o.groundWeight = (d == dGroundPlane) ? 1.0f : 0.0f;


	uint maxSteps;
	float maxDistance, surfaceDistance;
	GetRayMarcherConfig(maxSteps, maxDistance, surfaceDistance);
	o.backgroundWeight = d > maxDistance ? 0.0f : 1.0f;
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
	o.color = vec3(0,0,0);
	vec3 normal = CalculateNormal(p);
	vec3 groundPlaneTexColor = texture(GroundTexture, p.xz * GroundTextureScale).rgb;
	vec3 groundColor = mix(groundPlaneTexColor, vec3(1,0,0), o.blackHoleImpact * 3);
	vec3 blackHoleColorXY = 
	   texture(BlackHoleTexture,normal.xy * BlackHoleTextureScale + vec2(- Time * 0.2 * isBlackHoleTextureMoving, 0) ).rgb;
	
	vec3 blackHoleColorFinal = blackHoleColorXY;

	float fresnelFactor = dot(normal, dir); 
	fresnelFactor = max(0, 1 - fresnelFactor);
	fresnelFactor = pow(fresnelFactor, FresnelPower) * FresnelStrength;
	
	vec3 blackHoleC = mix(blackHoleColorFinal, FresnelColor, fresnelFactor * isFresnelActive);
	vec3 blackHoleParticleC = mix(blackHoleColorFinal, FresnelColor, fresnelFactor * (BlackHoleRadius / BlackHoleParticlesRadius) * isFresnelActive);
	vec3 blackHoleColor = mix(blackHoleC, blackHoleParticleC, 0.1);
	o.color  = mix(blackHoleColor, groundColor, o.groundWeight);
	vec3 viewDir = normalize(-p);
	float dotNV = dot(normalize(-p), normal);
	return vec4(dotNV * o.color, 1.0f);
}