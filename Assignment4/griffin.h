#pragma once

#include <string>
#include <vector>
#include <math.h>
#include "model.h"

class Griffin{
	std::vector<Model> models;

	public:
		Griffin(const char* body_path, const char* left_wing_path, const char* right_wing_path, GLuint shaderProgramID);
		void Draw(float deltaTime);

	private:
		float animationTime;
};