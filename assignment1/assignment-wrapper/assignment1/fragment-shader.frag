// Minimal fragment shader
// Iain Martin 2018

#version 420 core

vec4 specular_colour = vec4(1.0, 0.8, 0.6, 1.0);
vec4 global_ambient = vec4(0.05, 0.05, 0.05, 1.0);
int shininess = 8;

//Inputs from V shadercallcoherent
in vec3 fnormal, flightdir, fposition;
in vec4 fdiffusecolour, fambientcolour;

uniform uint attenuationmode;
uniform uint emitmode;

out vec4 outputColour;
void main()
{
	//Create a vec4 (0,0,0) for the emissive light, set to 0 unless emitmode is true
	vec4 emissive = vec4(0);
	vec4 fambientcolour = fdiffusecolour * 0.2;
	vec4 fspecularcolour = vec4(1.0, 0.8, 0.6, 1.0);
	float distancetolight = length(flightdir);

	//Normalise interpolated vextors
	vec3 L = normalize(flightdir);
	vec3 N = normalize(fnormal);

	//Calculate diffuse component
	vec4 diffuse = max(dot(N,L), 0.0) * fdiffusecolour;

	//Calculate diffuse component
	vec3 V = normalize(-fposition);
	vec3 R = reflect(-L, N);
	vec4 specular = pow(max(dot(R,V), 0.0), shininess) * fspecularcolour;

	//Calculate attenuation factor, 
	//Turn off attenuation if attenuationmode is not set to true
	float attenuation;
	if(attenuationmode != 1)	attenuation = 1.0;

	else
	{
		//Define attenuation constatns
		float attenuation_k1 = 0.5;
		float attenuation_k2 = 0.5;
		float attenuation_k3 = 0.5;
		attenuation = 1.0 / (attenuation_k1 + attenuation_k2 * distancetolight + attenuation_k3 * pow(distancetolight, 2));
	}

	//If emitemode is enable, turn on emissive lighting
	if (emitmode == 1) emissive = vec4(1.0, 1.0, 0.8, 1.0);

	//Calculate output colour, based on the attenuation, diffuse and specular components
	outputColour = attenuation * (diffuse + specular * 0.6) + emissive + global_ambient + fambientcolour;
}

//in vec4 fcolour;
//out vec4 outputColor;
//void main()
//{
//	outputColor = fcolour;
//}