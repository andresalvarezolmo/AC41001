/*
 assignment1.cpp
 Demonstrates fragment lighting with attenuation 
 with per-pixel lighting (Phong-shading) coded in the fragment shader
 Displays a series of cylinders, squares, spheres and cubes that combined form a turntable
 This class includes controls to move the light source, the point of view, the entire turntable, the dial and to change the colour of the light
 Andres Alvarez Olmo 2021
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
GLuint modelID, viewID, projectionID, lightposID, normalmatrixID, bulbpositionID;
GLuint colourmodeID, emitmodeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

GLfloat rotation_angle;	//lateral rotation stick
GLfloat rotation_lift;	//vertical rotation stick	

GLfloat disk_rotation_angle; //disk rotation angle 

GLfloat dial_rotation_angle; //dial rotaion angle


GLuint numspherevertices;

/* Global instances of our objects */
Sphere stickSphere;
Sphere bulbSphere;
Sphere aSphere;
Cube aCube;
Square aSquare;
Cylinder bigCylinder(glm::vec3(0.0f, 0.0f, 0.0f));
Cylinder smallCylinder(glm::vec3(1.0f, 0.0f, 0.0f));
Cylinder tube(glm::vec3(1.0f, 1.0f, 1.0f));
Cylinder dial(glm::vec3(0.66f, 0.66f, 0.66f));

using namespace std;
using namespace glm;


void init(GLWrapper* glw)
{

	mat4 m1 = translate(mat4(1.0f), vec3(1.f, 0.f, 2.f));
	vec4 v1 = vec4(-1.f, 2.f, 1.f, 1.f);
	vec4 v2 = m1 * v1;

	vec4 specular_colour[] = {
		vec4(0.0, 0.2, 0.9, 1.0),
		vec4(1.0, 0.8, 0.6, 1.0),
		vec4(0.0, 0.9, 0.6, 1.0),
		vec4(0.8, 0.1, 0.3, 1.0),
		vec4(0.5, 0.0, 0.6, 1.0),
		vec4(0.0, 0.7, 0.0, 1.0),
	};

	/* Set the object transformation controls to their initial values */
	rotation_angle = 0.0f;
	rotation_lift = 0.0f;
	speed = 0.05f;
	x = 0;
	y = 0;
	z = 0;
	vx = 0; vx = 0, vz = 0.f;
	light_x = 0.5f; light_y = 0.05f; light_z = 0.95f;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	model_scale = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 1; emitmode = 0;
	numlats = 40;
	numlongs = 40;		
	disk_rotation_angle = 0.0;
	dial_rotation_angle = 0.0;

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
	viewID = glGetUniformLocation(program, "view");
	projectionID = glGetUniformLocation(program, "projection");
	lightposID = glGetUniformLocation(program, "lightpos");
	normalmatrixID = glGetUniformLocation(program, "normalmatrix");

	/* create objects */
	aSquare.makeSquare();
	aSphere.makeSphere(numlats, numlongs, vec3(1.0, 1.0, 1.0));
	bulbSphere.makeSphere(numlats, numlongs, vec3(1.0, 0.647, 0.0));
	stickSphere.makeSphere(numlats, numlongs, vec3(1.0, 0.0, 0.0));
	aCube.makeCube();
	bigCylinder.makeCylinder(true);
	smallCylinder.makeCylinder(false);
	tube.makeCylinder(false);
	dial.makeCylinder(true);
}

void display()
{
	glClearColor(0.0f, 0.0f, 0.1f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);

	stack<mat4> model;
	model.push(mat4(1.0f));

	mat3 normalmatrix;

	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, -3, 2.3), // Camera is at (0,-3,2.3), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Apply rotations to the view position. This wil get appledd to the whole scene
	view = rotate(view, -radians(vx), vec3(1, 0, 0));
	view = rotate(view, -radians(vy), vec3(0, 1, 0));
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);


	glUniform1ui(colourmodeID, colourmode);
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
		model.top() = scale(model.top(), vec3(3, 3, 0.5));

		// Send the model uniform and normal matrix to the currently bound shader,
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		// Recalculate the normal matrix and send to the vertex shader
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our cube*/
		aCube.drawCube(drawmode);

	}
	model.pop();

	// This block of code draws the custom square
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x - 0.59f, y + 0.59f, z + 0.14));
		model.top() = scale(model.top(), vec3(0.3, 0.3, 0.05));//scale equally in all axis

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		aSquare.drawSquare(drawmode);
	}
	model.pop();

	// This block of code draws the volume dial
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x - 0.59f, y - 0.59f, z + 0.14));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = rotate(model.top(), radians(dial_rotation_angle), vec3(0, 1, 0));
		model.top() = scale(model.top(), vec3(0.1f, 0.1f, 0.1f)); 

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		dial.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the bigger black disk
	model.push(model.top());
	{		
		model.top() = translate(model.top(), vec3(x - 0.08, y, z + 0.15));

		//check if stick is on the right position, if it is rotate the disk
		if (rotation_angle <= -17.5 && rotation_angle >= -40 && rotation_lift == 0) disk_rotation_angle -= 0.1;

		model.top() = rotate(model.top(), radians(disk_rotation_angle), vec3(0, 0, 1));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.59f, 0.03f, 0.59f));

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw the big black disk*/
		bigCylinder.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the smaller red disk
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x - 0.08f, y, z + 0.165f));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));//scale equally in all axis
		model.top() = scale(model.top(), vec3(0.2f, 0.022f, 0.2f));//scale equally in all axis

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw our small red disj*/
		smallCylinder.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the cylinder (tube shape) that support the disk
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x - 0.08, y, z + 0.19));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.02f, 0.06f, 0.02f));

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw the tube*/
		tube.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the small central cylinder of the disk
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x + 0.6, y + 0.58, z + 0.16));
		model.top() = rotate(model.top(), radians(90.0f), vec3(1, 0, 0));
		model.top() = scale(model.top(), vec3(0.04f, 0.3f, 0.04f));
		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw the small center cylinder*/
		tube.drawCylinder(drawmode);
	}
	model.pop();

	// This block of code draws the stick
	model.push(model.top());
	{
		model.top() = translate(model.top(), vec3(x + 0.6f, y + 0.6f, z + 0.275f));

		//Define stick rotation boundaries, if stick is on top of the disk then move it at the same pace as the track is rotation
		if (rotation_angle <= -17.5 && rotation_angle >= -40 && rotation_lift == 0) rotation_angle -= 0.0025;

		//stick rotations, applied in inverse to match the mathematical restrictions
		model.top() = rotate(model.top(), radians(rotation_angle), vec3(0, 0, 1));
		model.top() = rotate(model.top(), radians(rotation_lift), vec3(1, 0, 0));
		model.top() = translate(model.top(), vec3(0, -0.6f, 0));

		model.top() = scale(model.top(), vec3(0.125f, 2.2f, 0.1f));

		glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));

		normalmatrix = transpose(inverse(mat3(view * model.top())));
		glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

		/* Draw the stick*/
		aCube.drawCube(drawmode);
		model.push(model.top());
		{
			//Draw the sphere connected to the stick without popping the previous transformation so the ball is also rotated around the same axis as the stick

			model.top() = translate(model.top(), vec3(0.6 - x, -0.51 - y, 0.22 - z));
			model.top() = translate(model.top(), vec3(x - 0.6, y + 0.275, z - 0.7));


			model.top() = scale(model.top(), vec3(1 / 25.f, 1 / 25.f, 1 / 25.f));
			model.top() = scale(model.top(), vec3(1 / 0.125f, 1 / 2.2f, 1 / 0.1f));

			glUniformMatrix4fv(modelID, 1, GL_FALSE, &(model.top()[0][0]));
			normalmatrix = transpose(inverse(mat3(view * model.top())));
			glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

			stickSphere.drawSphere(drawmode);
		}
		model.pop();
	}
	model.pop();

	glDisableVertexAttribArray(0);
	glUseProgram(0);

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

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Z' && x > -0.3) x -= speed;
	if (key == 'X' && x < 0.3) x += speed;
	if (key == 'C' && y > -0.3) y -= speed;
	if (key == 'V' && y < 0.3) y += speed;
	if (key == 'B' && z > -0.3) z -= speed;
	if (key == 'N' && z < 0.3) z += speed;
	if (key == '1' && light_x > -1.1) light_x -= speed;
	if (key == '2' && light_x < 1.1) light_x += speed;
	if (key == '3' && light_y > -0.75) light_y -= speed;
	if (key == '4' && light_y < 0.75) light_y += speed;
	if (key == '5' && light_z > 0.25) light_z -= speed;
	if (key == '6' && light_z < 1) light_z += speed;
	if (key == '7' && vx > -8.f) vx -= 1.f;
	if (key == '8' && vx < 8.f) vx += 1.f;
	if (key == 'O') vz -= 1.f;
	if (key == 'P') vz += 1.f;

	if (key == 'D') {
		if (rotation_angle < 0) {
			rotation_angle += 1.25f;
		}
	}
	if (key == 'A') {
		if (rotation_angle > -25) {
			rotation_angle -= 1.25f;
		}
	}
	if (key == 'W') {
		if (rotation_lift > -6.25f) {
			rotation_lift -= 1.25f;
		}
	}
	if (key == 'S') {
		if (rotation_lift < 0.0f) {
			rotation_lift += 1.25f;
		}
	}

	if (key == 'U') {
		if (dial_rotation_angle < 0) {
			dial_rotation_angle += 1.f;
		}
	}

	if (key == 'I') {
		if (dial_rotation_angle <= 0 && dial_rotation_angle > -360) {
			dial_rotation_angle -= 1.f;
		}
	}

	if (key == ' ' && action != GLFW_PRESS)
	{
		colourmode = colourmode++ % 4;
	}

}

void displayControls() {

	cout << "OBSERVATIONS: \n" << endl;
	cout << "\t- Place stick on top of the track so the song can start, once the \n\tstick reaches the end of the track, the disk will stop spinning\n" << endl;
	cout << "\t- Controls are limited so objects can not too far from the scene\n" << endl;

	cout << "CONTROL KEYS: \n" << endl;

	cout << "\t- W, S -> Lift/Lower stick" << endl;
	cout << "\t- A, D -> Rotate stick clockwise / anticlockwise\n" << endl;

	cout << "\t- U, I-> Rotate dial clockwise / anticlockwise\n" << endl;

	cout << "\t- 1,2 -> Move light in X axis" << endl;
	cout << "\t- 3,4 -> Move light in Y axis" << endl;
	cout << "\t- 5,6 -> Move light in Z axis\n" << endl;

	cout << "\t- 7,8 -> Move scene on x axis;" << endl;
	cout << "\t- O,P -> Move scene on z axis;\n" << endl;

	cout << "\t- Z, X -> Move object in X axis;" << endl;
	cout << "\t- C, V -> Move object in Y axis;" << endl;
	cout << "\t- B, N -> Move object in Z axis;\n" << endl;

	cout << "\t- SPACE -> Colour mode" << endl;
	cout << "\t- ESC -> Terminate program" << endl;
}

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

	displayControls();

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}



