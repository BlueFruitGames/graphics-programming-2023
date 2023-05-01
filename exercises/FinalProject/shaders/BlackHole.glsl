// Uniforms
// Replace constants with uniforms with the same name

#define PI 3.1415926538

//Bend Uniforms
uniform vec2 BendDistanceBounds = vec2(-1,1);
uniform vec3 BendOrigin = vec3(0, 0, 0);

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
	float dBlackhole = SphereSDF(TransformToLocalPoint(p, spherePosition), BlackHoleRadius + sin(GroundSpeed * Time) * 0.25);
	return dBlackhole;
}

float createGround(vec3 p, BlackHoleInfo blackHoleInfo, out vec3 color){
	float blackHoleImpact;
	float dGroundPlane = BendedPlaneSDF(p, GroundNormal, GroundOffset,
	BendOrigin, BendDistanceBounds,	blackHoleInfo, Time * GroundSpeed, blackHoleImpact);

	vec3 groundPlaneTexColor = texture(GroundTexture, p.xz * GroundTextureScale).rgb;
	
	
	color = mix(groundPlaneTexColor, vec3(1,0,0), blackHoleImpact);
	return dGroundPlane;
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
	vec3 blackHolePosition = BlackHoleStartPosition + vec3(ViewMatrix * vec4(BendOrigin, 1.0f));
	BlackHoleInfo blackHoleInfo;
	blackHoleInfo.influence = BlackHoleInfluence;
	blackHoleInfo.position = blackHolePosition;
	blackHoleInfo.influenceBounds = BlackHoleInfluenceBounds;
	blackHoleInfo.radius = BlackHoleRadius;

	//Creation of objects
	vec3 groundInfluencedColor;
	float dGroundPlane = createGround(p, blackHoleInfo, groundInfluencedColor);
	float dBlackhole = createBlackHole(p, blackHolePosition);
	float dBlackholeParticles = createBlackholeParticles(p, blackHolePosition);
	
	
	//Blending of colors
	float blend;
	float d = SmoothUnion(dBlackhole, dBlackholeParticles, Smoothness, blend);
	vec3 blackHoleColor = mix(texture(BlackHoleTexture,p.xy * BlackHoleTextureScale).rgb, vec3(0.4), blend);
	
	d = Union(d, dGroundPlane);
	o.color  = (d == dGroundPlane) ? groundInfluencedColor : blackHoleColor;
	return d;
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