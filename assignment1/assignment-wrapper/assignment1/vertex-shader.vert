// Vertex shader demonstrating a positional light
// source with attenuation
// Iain Martin 2018


// Specify minimum OpenGL version
#version 420 core


// Define the vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

// Outputs to send to the fragment shader
out vec3 fnormal;
out vec3 flightdir, fposition;
out vec4 fdiffusecolour;

//Uniforms defined in the application
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode;
uniform vec4 lightpos;

void main()
{
	vec4 position_h = vec4(position, 1.0);	// Convert the (x,y,z) position to homogeneous coords (x,y,z,w)
	vec3 light_pos3 = lightpos.xyz;

	fdiffusecolour = colour;

	mat4 mv_matrix = view * model;
	fposition = (mv_matrix * position_h).xyz;
	fnormal = normalize(normalmatrix* normal);
	flightdir = light_pos3 - fposition;

	gl_Position = (projection * view * model) * position_h;
}