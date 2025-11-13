#pragma once

#include <string>
#include <vector>
#include <math.h>
#include "model.h"
#include "maths_funcs.h"

struct GriffinFiles {
	const char* body_path;
	const char* left_wing_path;
	const char* right_wing_path;
};

class Griffin{
	std::vector<Model> models;

	public:
		Griffin(GriffinFiles file_paths, vec3 position, float FlightRadius, float flightSpeed, GLuint shaderProgramID);
		void Draw(float deltaTime);

	private:
		float animationTime;
		vec3 spawnPosition;
		float flightRadius;
		float flightSpeed;
};