/* cube.h
 Example class to create a generic cube object
 Iain Martin October 2018
*/

#include "square.h"

/* I don't like using namespaces in header files but have less issues with them in
seperate cpp files */
using namespace std;

/* Define the vertex attributes for vertex positions and normals.
Make these match your application and vertex shader
You might also want to add colours and texture coordinates */
Square::Square()
{
	attribute_v_coord = 0;
	attribute_v_colours = 1;
	attribute_v_normal = 2;
	numvertices = 6;
}


Square::~Square()
{
}


/* Make a cube from hard-coded vertex positions and normals  */
void Square::makeSquare()
{
	/* Define vertices for a square in 2 triangles */
	glm::vec3 vertexPositions[] = {
		glm::vec3(-0.25f, 0.25f, -0.25f),
		glm::vec3(-0.25f, -0.25f, -0.25f),
		glm::vec3(0.25f, -0.25f, -0.25f),

		glm::vec3(0.25f, -0.25f, -0.25f),
		glm::vec3(0.25f, 0.25f, -0.25f),
		glm::vec3(-0.25f, 0.25f, -0.25f)
	};

	//GLfloat vertexPositions[] =
	//{
	//	-0.25f, 0.25f, -0.25f,
	//	-0.25f, -0.25f, -0.25f,
	//	0.25f, -0.25f, -0.25f,

	//	0.25f, -0.25f, -0.25f,
	//	0.25f, 0.25f, -0.25f,
	//	-0.25f, 0.25f, -0.25f,
	//};

	/* Manually specified colours for our cube */
	float vertexColours[] = {
		//0.55f, 0.27f, 0.07f, 1.0f,
		//0.55f, 0.27f, 0.07f, 1.0f,
		//0.55f, 0.27f, 0.07f, 1.0f,
		//0.55f, 0.27f, 0.07f, 1.0f,
		//0.55f, 0.27f, 0.07f, 1.0f,
		//0.55f, 0.27f, 0.07f, 1.0f,
		1.0f, 1.0f, 1.0, 1.0f,
		1.0f, 1.0f, 1.0, 1.0f,
		1.0f, 1.0f, 1.0, 1.0f,
		1.0f, 1.0f, 1.0, 1.0f,
		1.0f, 1.0f, 1.0, 1.0f,
		1.0f, 1.0f, 1.0, 1.0f,
	};

	/* Manually specified normals for our cube */
	//GLfloat normals[] =
	//{
	//	0, 0, -1.f, 0, 0, -1.f, 0, 0, -1.f,
	//	0, 0, -1.f, 0, 0, -1.f, 0, 0, -1.f,
	//};

	glm::vec3 normals[12];

	for (int v = 0; v < numvertices; v += 3)
	{
		glm::vec3 normal = glm::cross(vertexPositions[v + 1] - vertexPositions[v],
			vertexPositions[v + 2] - vertexPositions[v]);
			normals[v] = normals[v + 1] = normals[v + 2] = normal;
	}

	/* Create the vertex buffer for the cube */
	glGenBuffers(1, &positionBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, numvertices * sizeof(glm::vec3), &vertexPositions[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the colours buffer for the cube */
	glGenBuffers(1, &colourObject);
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColours), vertexColours, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Create the normals  buffer for the cube */
	glGenBuffers(1, &normalsBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), &normals[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/* Draw the cube by bining the VBOs and drawing triangles */
void Square::drawSquare(int drawmode)
{
	/* Bind cube vertices. Note that this is in attribute index attribute_v_coord */
	glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
	glEnableVertexAttribArray(attribute_v_coord);
	glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index attribute_v_colours */
	glBindBuffer(GL_ARRAY_BUFFER, colourObject);
	glEnableVertexAttribArray(attribute_v_colours);
	glVertexAttribPointer(attribute_v_colours, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube normals. Note that this is in attribute index attribute_v_normal */
	glEnableVertexAttribArray(attribute_v_normal);
	glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
	glVertexAttribPointer(attribute_v_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glPointSize(3.f);

	// Switch between filled and wireframe modes
	if (drawmode == 1)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Draw points
	if (drawmode == 2)
	{
		glDrawArrays(GL_POINTS, 0, numvertices * 3);
	}
	else // Draw the cube in triangles
	{
		glDrawArrays(GL_TRIANGLES, 0, numvertices * 3);
	}
}