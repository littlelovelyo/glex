#version 450 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNorm;
layout(location=2) in vec2 aTexCoords;

layout(set=1, binding=0) uniform Material
{
	vec3 diffuse;
	float specular;
	float shininess;
} material;

void main()
{
	gl_Position = vec4(aPos, 1.0);
}