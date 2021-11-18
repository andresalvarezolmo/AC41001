/*
 poslight.cpp
 Demonstrates a positional light with attenuation
 with per-vertex lighting (Gouraud shading) coded in the vertex shader.
 Displays a cube and a sphere and a small sphere to show the light position
 Includes controls to move the light source and rotate the view
 Iain Martin October 2018
*/

/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#ifdef _DEBUG
#pragma comment(lib, "glfw3D.lib")
#pragma comment(lib, "glloadD.lib")
#else
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glload.lib")
#endif
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>
#include <stack>

   /* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include headers for our objects
#include "sphere.h"
#include "cube.h"
#include "cylinder.h"
#include "square.h"

/* Define buffer object indices */
GLuint elementbuffer;

GLuint program;		/* Identifier for the shader prgoram */
GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */
GLuint emitmode;
GLuint attenuationmode;

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, model_scale, z, y, vx, vy, vz;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;

GLfloat sphere_x, sphere_y, sphere_z;

GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLuint numlats, numlongs;	//Define the resolution of the sphere object
GLfloat speed;				// movement increment

GLfloat light_x;
GLfloat light_y;
GLfloat light_z;

/* Uniforms*/
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID;
GLuint colourmodeID, emitmodeID, attenuationmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

GLfloat rotation_angle;	//lateral rotation disk
GLfloat rotation_lift;	//vertical rotation disk	

GLfloat disk_rotation_angle;

GLuint numspherevertices;

/* Global instances of our objects */
Sphere aSphere;
Cube aCube;
Square aSquare;
Cylinder bigCylinder(glm::vec3(0.0f, 0.0f, 0.0f));
Cylinder smallCylinder(glm::vec3(1.0f, 0.0f, 0.0f));
Cylinder tube(glm::vec3(1.0f, 1.0f, 1.0f));

using namespace std;
using namespace glm;


/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper* glw)
{

	mat4 m1 = translate(mat4(1.0f), vec3(1.f, 0.f, 2.f));
	vec4 v1 = vec4(-1.f, 2.f, 1.f, 1.f);
	vec4 v2 = m1 * v1;
	cout << "v2 " << v2.x << ", " << v2.y << ", " << v2.z << ", " << v2.w << endl;



	/* Set the object transformation controls to their initial values */
	rotation_angle = 0.0f;
	rotation_lift = 0.0f;
	speed = 0.05f;
	x = 0.05f;
	y = 0;
	z = 0;
	vx = 0; vx = 0, vz = 4.f;
	light_x = 0.75f; light_y = 0.45f; light_z = 0.65f;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1; emitmode = 0;
	attenuationmode = 1; // Attenuation is on by default
	numlats = 40;		// Number of latitudes in our sphere
	numlongs = 40;		// Number of longitudes in our sphere

	disk_rotation_angle = 0.0;

	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and build the vertex and fragment shaders */
	try
	{
		program = glw->LoadShader("vertex-shader.vert", "fragment-shader.frag");
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	/* Define uniforms to send to vertex shader */
	modelID = glGetUniformLocation(program, "model");
	colourmodeID = glGetUniformLocation(program, "colourmode");
	emitmodeID = glGetUniformLocation(program, "emitmode");
	attenuationmodeID = glGetUniformLocation(program, "attenuationmode");
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	lightposID = glGetUniformLocation(program, "lightpos");
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");

	/* create our sphere and cube objects */
	aSquare.makeSquare();
	aSphere.makeSphere(numlats, numlongs, vec3(0.0,0.0,1.0));
	aCube.makeCube();
	bigCylinder.makeCylinder(true);
	smallCylinder.makeCylinder(false);
	tube.makeCylinder(false);
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	/* Make the compiled shader program current */
	glUseProgram(program);

	// Define our model transformation in a stack and 
	// push the identity matrix onto the stack
	stack<mat4> model;
	model.push(mat4(1.0f));

	// Define the normal matrix
	mat3 normalmatrix;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, -3, 2), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Apply rotations to the view position. This wil get appleid to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);

	// Send our projection and view uniforms to the currently bound shader
	// I do that here because they are the same for all objects
	glUniform1ui(colourmodeID, colourmode);
	glUniform1ui(attenuationmodeID, attenuationmode);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform4fv(lightposID, 1, value_ptr(lightpos));

	/* Draw a small sphere in the lightsource position to visually represent the light source */
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(light_x, light_y, light_z));
		model.top() = scale(model.top(), vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
																	 // Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our lightposition sphere  with emit mode on*/
		emitmode = 1;
		glUniform1ui(emitmodeID, emitmode);
		aSphere.drawSphere(drawmode);
		emitmode = 0;
		glUniform1ui(emitmodeID, emitmode);
	}
	model.pop();

	// Define the global model transformations (rotate and scale). Note, we're not modifying thel ight source position
	model.top() = scale(model.top(), vec3(model_scale, model_scale, model_scale));//scale equally in all axis
	model.top() = rotate(model.top(), -radians(angle_x), glm::vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model.top() = rotate(model.top(), -radians(angle_y), glm::vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model.top() = rotate(model.top(), -radians(angle_z), glm::vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// This block of code draws the cube
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(x, y, z));
		model.top() = scale(model.top(), vec3(3, 3, 0.5));//scale equally in all axis
		//model.top() = scale(model.top(), vec3(3*model_scale, 3*model_scale, model_scale/2));//scale equally in all axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);

	}
	model.pop();	
	
	// This block of code draws the square
	model.push(model.top());
	{
		// Define the model transformations for the square
		model.top() = translate(model.top(), vec3(x-0.59f, y-0.59f, z + 0.14));
		model.top() = scale(model.top(), vec3(0.3, 0.3, 0.05));//scale equally in all axis
		//model.top() = scale(model.top(), vec3(3*model_scale, 3*model_scale, model_scale/2));//scale equally in all axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

	//	// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aSquare.drawSquare(drawmode);
	}
	model.pop();

	// This block of code draws the bigger disk
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(x - 0.08, y, z + 0.15));
		if (rotation_angle == -30 && rotation_lift == 0) disk_rotation_angle += 0.1;
		model.top() = rotate(model.top(), radians(disk_rotation_angle), vec3(0, 0, 1));//scale equally in all axis
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));//scale equally in all axis
		model.top() = scale(model.top(), vec3(0.59f, 0.03f, 0.59f));//scale equally in all axis
		//model.top() = scale(model.top(), vec3(model_scale/1.7, model_scale / 25, model_scale/1.7));//scale equally in all axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		bigCylinder.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the smaller disk
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(x - 0.08f, y, z + 0.165f));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));//scale equally in all axis
		model.top() = scale(model.top(), vec3(0.2f, 0.022f, 0.2f));//scale equally in all axis
		//model.top() = scale(model.top(), vec3(model_scale / 5, model_scale / 25, model_scale / 5));//scale equally in all axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		smallCylinder.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the tube cylinder of the disk
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(x - 0.08, y, z + 0.19));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));//scale equally in all axis
		model.top() = scale(model.top(), vec3(0.02f, 0.06f, 0.02f));//scale equally in all axis
		//model.top() = scale(model.top(), vec3(model_scale / 50, model_scale / 10, model_scale / 50));//scale equally in all axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		tube.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the tube cylinder of the disk
	model.push(model.top());
	{
		// Define the model transformations for the cube
		model.top() = translate(model.top(), vec3(x + 0.6, y + 0.58, z + 0.16));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));//scale equally in all axis
		model.top() = scale(model.top(), vec3(0.04f, 0.3f, 0.04f));//scale equally in all axis
		//model.top() = scale(model.top(), vec3(model_scale / 50, model_scale / 10, model_scale / 50));//scale equally in all axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		tube.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the stick
	model.push(model.top());
	{
		// Define the model transformations for the cube
		//model.top() = translate(model.top(), vec3(x + 0.6f, y + 0.6f, z + 0.26f));
		//model.top() = translate(model.top(), vec3(x + 0.6f, y + 0.6f, z + 0.235f));
		model.top() = translate(model.top(), vec3(x + 0.6f, y + 0.6f, z + 0.275f));

		//for stick rotation
		model.top() = rotate(model.top(), radians(rotation_angle), vec3(0, 0, 1));
		//Rotate up and down
		model.top() = rotate(model.top(), radians(rotation_lift), vec3(1, 0, 0));
		model.top() = translate(model.top(), vec3(0, -0.6f, 0));

		model.top() = scale(model.top(), vec3(0.125f, 2.2f, 0.1f));//scale equally in all axis

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);
		model.push(model.top());
		{
			model.top() = translate(model.top(), vec3(0.6-x, -0.51-y, 0.22-z));
			model.top() = translate(model.top(), vec3(x - 0.6, y + 0.275, z - 0.7));

			//model.top() = translate(model.top(), vec3(x + sphere_x, y + sphere_y - 0.6, z + sphere_z));

			model.top() = scale(model.top(), vec3(1 / 25.f, 1 / 25.f, 1 / 25.f));//scale equally in all axis
			model.top() = scale(model.top(), vec3(1/0.125f, 1/2.2f, 1/0.1f));//scale equally in all axis

			//model.top() = scale(model.top(), vec3(model_scale /20.f, model_scale / 20.f, model_scale / 20.f));//scale equally in all axis

			// Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

			aSphere.drawSphere(drawmode); // Draw our sphere

		//	model.top() = translate(model.top(), vec3(x + 0.5f, y - 0.7, z+0.2));
		//	model.top() = translate(model.top(), vec3(x + sphere_x, y + sphere_y, z + sphere_z));
		//	model.top() = scale(model.top(), vec3(1 / 25.f, 1 / 25.f, 1 / 25.f));//scale equally in all axis
		//	model.top() = scale(model.top(), vec3(model_scale /20.f, model_scale / 20.f, model_scale / 20.f));//scale equally in all axis

		// //Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
		//	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		//	normalmatrix = transpose(inverse(mat3(view * model.top())));
		//	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		//aSphere.drawSphere(drawmode); // Draw our sphere
		}
		model.pop();
	}
	model.pop();

	// This block of code draws the sphere
	//model.push(model.top());
	//{
	//	model.top() = translate(model.top(), vec3(x + sphere_x, y + sphere_y, z + sphere_z));
	//	//for stick rotation
	//	model.top() = rotate(model.top(), radians(rotation_angle), vec3(0, 0, 1));
	//	//Rotate up and down
	//	model.top() = rotate(model.top(), radians(rotation_lift), vec3(1, 0, 0));

	//	//model.top() = translate(model.top(), vec3(x + sphere_x, y + sphere_y - 0.6, z + sphere_z));

	//	model.top() = scale(model.top(), vec3(1 / 25.f, 1 / 25.f, 1 / 25.f));//scale equally in all axis
	//	//model.top() = scale(model.top(), vec3(model_scale /20.f, model_scale / 20.f, model_scale / 20.f));//scale equally in all axis

	//	// Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
	//	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
	//	normalmatrix = transpose(inverse(mat3(view * model.top())));
	//	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	//	aSphere.drawSphere(drawmode); // Draw our sphere

	////	model.top() = translate(model.top(), vec3(x + 0.5f, y - 0.7, z+0.2));
	////	model.top() = translate(model.top(), vec3(x + sphere_x, y + sphere_y, z + sphere_z));
	////	model.top() = scale(model.top(), vec3(1 / 25.f, 1 / 25.f, 1 / 25.f));//scale equally in all axis
	////	model.top() = scale(model.top(), vec3(model_scale /20.f, model_scale / 20.f, model_scale / 20.f));//scale equally in all axis

	//// //Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
	////	glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
	////	normalmatrix = transpose(inverse(mat3(view * model.top())));
	////	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	////aSphere.drawSphere(drawmode); // Draw our sphere
	//}
	//model.pop();



	glDisableVertexAttribArray(0);
	glUseProgram(0);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f * 4.f) / ((float)h / 480.f * 3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc_x -= speed;
	if (key == 'W') angle_inc_x += speed;
	if (key == 'E') angle_inc_y -= speed;
	if (key == 'R') angle_inc_y += speed;
	if (key == 'T') angle_inc_z -= speed;
	if (key == 'Y') angle_inc_z += speed;
	if (key == 'A') model_scale -= speed / 0.5f;
	if (key == 'S') model_scale += speed / 0.5f;
	if (key == 'Z') x -= speed;
	if (key == 'X') x += speed;
	if (key == 'C') y -= speed;
	if (key == 'V') y += speed;
	if (key == 'B') z -= speed;
	if (key == 'N') z += speed;
	if (key == '1') light_x -= speed;
	if (key == '2') light_x += speed;
	if (key == '3') light_y -= speed;
	if (key == '4') light_y += speed;
	if (key == '5') light_z -= speed;
	if (key == '6') light_z += speed;
	if (key == '7') vx -= 1.f;
	if (key == '8') vx += 1.f;
	if (key == '9') vy -= 1.f;
	if (key == '0') vy += 1.f;
	if (key == 'O') vz -= 1.f;
	if (key == 'P') vz += 1.f;	
	

	if (key == 'U') disk_rotation_angle -= 0.5;
	if (key == 'I') disk_rotation_angle += 0.5;


	if (key == 'L') {
		if (rotation_angle < 0) {
			rotation_angle += 5.0f;
		}
	}
	if (key == 'K') {
		if (rotation_angle > -30) {
			rotation_angle -= 5.0f;
		}
	}
	if (key == 'J') {
		if (rotation_lift > -6.25f) {
			rotation_lift -= 1.25f;
		}
	}
	if (key == 'H') {
		if (rotation_lift < 0.0f) {
			rotation_lift += 1.25f;
		}
	}

	//cout << "view x" << vx << endl;
	//cout << "view y" << vz << endl;
	//cout << "view z" << vy << endl;	
	 
	//cout << "sphere x" << sphere_x<< endl;
	//cout << "sphere y" << sphere_y<< endl;
	//cout << "sphere z" << sphere_z<< endl;


	//cout << "light x" << light_x << endl;
	//cout << "light y" << light_y << endl;
	//cout << "light z" << light_z << endl;
	//cout << "rotation lift" << rotation_lift << endl;

	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Turn attenuation on and off */
	if (key == '.' && action != GLFW_PRESS)
	{
		attenuationmode = !attenuationmode;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == ',' && action != GLFW_PRESS)
	{
		drawmode++;
		if (drawmode > 2) drawmode = 0;
	}
}


/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper* glw = new GLWrapper(1024, 768, "Position light example");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	/* Output the OpenGL vendor and version */
	glw->DisplayVersion();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}

