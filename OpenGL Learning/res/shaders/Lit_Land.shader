#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
uniform vec4 u_Plane;

void main()
{
	gl_ClipDistance[0] = dot(vec4(position, 1.0), u_Plane);
	FragPos = vec3(u_Model * vec4(position, 1.0));
	Normal = mat3(transpose(inverse(u_Model))) * normal;
	TexCoords = texCoords;

	gl_Position = u_Projection * u_View * vec4(FragPos, 1.0);
};

#shader fragment
#version 330 core

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shininess;
};
struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

layout(location = 0) out vec4 color;

uniform vec3 u_ViewPos;
uniform Material u_Material;
uniform DirLight u_DirLight;
uniform SpotLight u_SpotLights[SPOT_LIGHTS_NUM];
uniform PointLight u_PointLights[POINT_LIGHTS_NUM];

uniform int u_PointLightsCount;
uniform int u_SpotLightsCount;

uniform sampler2D peakTexture;
uniform sampler2D mountainTexture;
uniform sampler2D middleTexture;
uniform sampler2D beachTexture;
uniform sampler2D deepTexture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;

uniform float peakHeight;
uniform float mountainHeight;
uniform float middleHeight;
uniform float beachHeight;
uniform float deepHeight;

uniform float u_Tiling;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

float map(float value, float min1, float max1, float min2, float max2) {
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}
float map01(float value, float min1, float max1) {
	return map(value, min1, max1, 0.0, 1.0);
}
float clamp01(float value) {
	return clamp(value, 0, 1);
}

void main()
{
	vec2 tiledTexCoords = TexCoords * u_Tiling;
	float height = texture(heightMap, TexCoords).r;
	float peak = clamp01(map(height, mountainHeight, peakHeight, 0, 1));
	float mountain = clamp01(map(height, middleHeight, mountainHeight, 0, 1));
	mountain -= clamp01(map(height, mountainHeight, peakHeight, 0, 1));
	float middle = clamp01(map(height, beachHeight, middleHeight, 0, 1));
	middle -= clamp01(map(height, middleHeight, mountainHeight, 0, 1));
	float beach = clamp01(map(height, deepHeight, beachHeight, 0, 1));
	beach -= clamp01(map(height, beachHeight, middleHeight, 0, 1));
	float deep = clamp01(map(height, beachHeight, deepHeight, 0, 1));

	vec4 peakColor = texture(peakTexture, tiledTexCoords)*peak;
	vec4 mountainColor = texture(mountainTexture, tiledTexCoords)*mountain;
	vec4 middleColor = texture(middleTexture, tiledTexCoords)*middle;
	vec4 beachColor = texture(beachTexture, tiledTexCoords)*beach;
	vec4 deepColor = texture(deepTexture, tiledTexCoords)*deep;

	vec4 landColor = peakColor + mountainColor + middleColor + beachColor + deepColor;

	// properties
	vec3 norm = normalize(Normal);
	vec3 normalMapColor = texture(normalMap, TexCoords).rgb;

	
	norm = normalize(vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.b * 2.0 - 1.0, normalMapColor.g * 2.0 - 1.0));
	norm = normalize(normalMapColor * 2.0 - 1.0);

	vec3 viewDir = normalize(u_ViewPos - FragPos);

	// phase 1: Directional lighting
	vec3 result = CalcDirLight(u_DirLight, norm, viewDir);

	// phase 2: Point lights
	for (int i = 0; i < u_PointLightsCount; i++)
		result += CalcPointLight(u_PointLights[i], norm, FragPos, viewDir);
	// phase 3: Spot lights
	for (int i = 0; i < u_SpotLightsCount; i++)
		result += CalcSpotLight(u_SpotLights[i], norm, FragPos, viewDir);

	color = landColor;
	color += vec4(result, 0.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	// combine results
	vec3 ambient = light.ambient * u_Material.ambient;
	vec3 diffuse = light.diffuse * diff * u_Material.diffuse;
	vec3 specular = light.specular * spec * u_Material.specular;

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance +
		light.quadratic * (distance * distance));
	// combine results
	vec3 ambient = light.ambient * u_Material.ambient;
	vec3 diffuse = light.diffuse * diff * u_Material.diffuse;
	vec3 specular = light.specular * spec * u_Material.specular;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	// diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	// specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	// attenuation
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
	// spotlight intensity
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	// combine results
	vec3 ambient = light.ambient * u_Material.ambient;
	vec3 diffuse = light.diffuse * diff * u_Material.diffuse;
	vec3 specular = light.specular * spec * u_Material.specular;

	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	return (ambient + diffuse + specular);
}