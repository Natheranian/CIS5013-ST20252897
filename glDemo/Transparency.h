#pragma once

#include "AIMesh.h"

class Transparency : public AIMesh {
	// Specific shader to render cylinder (and it's effect)
	GLuint shader = 0;
	GLint shader_mvpMatrix = -1;

public:
Transparency(std::string filename, GLuint meshIndex = 0);

void render(glm::mat4 transform);

};