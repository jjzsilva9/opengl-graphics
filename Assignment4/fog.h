#pragma once

// Standard library
#include <string>
#include <vector>
#include <math.h>

// Project includes - needed for definitions
#include "maths_funcs.h"
#include "shader.h"
#include "directionallight.h"

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
	DirectionalLight* lightSource;
	GLuint shaderProgramID;
	bool enabled;

	Fog(vec4 color, float maxdist, float mindist, FogFactor factor, GLuint shaderProgramID, DirectionalLight* lightSource, bool enabled = true);
	void Update();
	void Draw();
private:
	vec4 getFogColor(float time);
};