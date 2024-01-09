
#include "Transparency.h"
#include "TextureLoader.h"
#include "shader_setup.h"

using namespace std;
using namespace glm;


Transparency::Transparency(std::string filename, GLuint meshIndex) : AIMesh(filename, meshIndex) {

	// Load shader
	shader = setupShaders(string("Assets\\Shaders\\TransparencyShader.vert"), string("Assets\\Shaders\\TransparencyShader.frag"));

	// Get uniform locations
	shader_mvpMatrix = glGetUniformLocation(shader, "mvpMatrix");

	// Set uniform values that will not change (ie. texture sampler values)
	glUseProgram(shader);
	glUseProgram(0); // restore default
}



void Transparency::render(mat4 transform) {

	// Handle cylinder effects internally so setup shader here
	glUseProgram(shader);
	glUniformMatrix4fv(shader_mvpMatrix, 1, GL_FALSE, (GLfloat*)&transform);

	AIMesh::render();
}