#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;

in vec4 fragPosLightSpace;

out vec4 fColor;

// lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// shadow map
uniform sampler2D shadowMap;



// fog
uniform bool fog;



vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float shadow;


// Function to compute directional light components
void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f); // Viewer is at the origin in eye coordinates
	
	// Transform normal
	vec3 normalEye = normalize(fNormal);	
	
	// Compute light direction
	vec3 lightDirN = normalize(lightDir);
	
	// Compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
		
	// Compute ambient light
	ambient = ambientStrength * lightColor;
	
	// Compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	// Compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}


float computeShadow() 
{
	float bias = 0.005f;
	// Perform perspective divide
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	normalizedCoords = normalizedCoords * 0.5 + 0.5;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	
	if (normalizedCoords.z > 1.0f)
		return 0.0f;

	float currentDepth = normalizedCoords.z;

	// Check if current pos in shadow
	return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
}

float computeFog() 
{
	float fogDensity = 0.012f; //higher value = denser and closer
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(0.9f, 0.35f, 0.0f); // Orange
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	// Compute shadow
	shadow = computeShadow();

	// Compute final color with directional light
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);

	
	// Fog effect
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	if (fog) {
		fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
	} else {
		fColor = vec4(color, 1.0f);
	}
} 