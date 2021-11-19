/* square.h
Class to to show a square implementation
Andres Alvarez Olmo
*/

#pragma once

#include "wrapper_glfw.h"
#include <vector>
#include <glm/glm.hpp>

class Square
{
public:
	
	Square();
	~Square();

	void makeSquare();
	void drawSquare(int drawmode);

	// Define vertex buffer object names 
	GLuint positionBufferObject;
	GLuint colourObject;
	GLuint normalsBufferObject;

	GLuint attribute_v_coord;
	GLuint attribute_v_normal;
	GLuint attribute_v_colours;

	int numvertices;

};
