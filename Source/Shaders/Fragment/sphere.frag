#version 450 core
layout(location=0) in vec3 vPos;
layout(location=1) in vec3 vCamPos;
layout(location=2) in vec2 vUV;

layout(set=1, binding=0) uniform sampler2D dayAndNight[2];

layout(location=0) out vec4 fColor;

const vec3 LIGHT_POS = vec3(0.0, 0.0, 3.0);

void main()
{
	vec3 normal = normalize(vPos);
	vec3 lightDir = normalize(LIGHT_POS - vPos);
	float diffuseStrength = max(0.6, dot(lightDir, normal));
	float specularStrength = pow(max(0.0, dot(normalize(reflect(vPos - vCamPos, normal)), lightDir)), 8);

	vec3 dayColor = texture(dayAndNight[0], vUV).rgb;
	vec3 nightColor = texture(dayAndNight[1], vUV).rgb;
	vec3 baseColor = mix(nightColor, dayColor, smoothstep(-0.1, 0.1, vPos.z));
	fColor = vec4(baseColor * diffuseStrength + specularStrength * vec3(1.0, 1.0, 1.0), 1.0);
}