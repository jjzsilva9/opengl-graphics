#pragma once

// Standard library
#include <string>
#include <vector>
#include <math.h>

// Project includes - needed for definitions
#include "maths_funcs.h"
#include "shader.h"

enum FogFactor {
	LINEAR,
	EXPONENTIAL,
	EXPONENTIAL_SQUARED
};
class Fog {
public:
	vec4 color;
	float maxdist;
	float mindist;
	FogFactor factor;
	bool dayCycle;
	GLuint shaderProgramID;

	Fog(vec4 color, float maxdist, float mindist, FogFactor factor, GLuint shaderProgramID, bool dayCycle = false);
	void Update(vec4 color, float maxdist, float mindist, FogFactor factor, bool dayCycle);
	void Draw(float deltaTime);
private:
	float timeOfDay;
	float cycleDuration;
	vec4 getFogColor(float time);
};