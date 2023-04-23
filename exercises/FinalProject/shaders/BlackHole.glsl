
// Uniforms
// Replace constants with uniforms with the same name

uniform float PlaneOffset = -5.0f;
uniform vec3 PlaneNormal = vec3(0, 1, 0);
uniform vec3 PlaneColor = vec3(1,1,1);
uniform vec2 BendDistanceBounds = vec2(-1,1);
uniform vec3 BendOrigin = vec3(0, 0, 0);

uniform vec3 SphereColor = vec3(0, 0, 1);
uniform vec3 SphereStartPosition = vec3(-2, 0, -10);
uniform float SphereRadius = 1.25f;

uniform float Time = 0.0f;
uniform float Speed = 3.0f;


// Output structure
struct Output
{
	// color of the closest figure
	vec3 color;
};


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
	
	float CurrentTime = Time * Speed;
	float dGroundPlane = BendedPlaneSDF(p, PlaneNormal, PlaneOffset, BendOrigin, BendDistanceBounds, CurrentTime);


	float dSphere = SphereSDF(TransformToLocalPoint(p, SphereStartPosition), SphereRadius);
	
	
	//o.color = mix(SphereColor, BoxColor, blend);
	//d = dGroundPlane;
	float d = Union(dSphere, dGroundPlane);
	o.color = d == dSphere ? SphereColor : PlaneColor;
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
