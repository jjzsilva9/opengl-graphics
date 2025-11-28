#pragma once

// Standard library
#include <string>
#include <vector>
#include <math.h>

// Project includes - needed for definitions
#include "maths_funcs.h"
#include "shader.h"
#include "directionallight.h"

DirectionalLight::DirectionalLight(vec4 position, vec3 diffuse, vec3 specular, vec3 ambient, GLuint shaderProgramID, bool dayCycle) {
	this->position = position;
	this->diffuse = diffuse;
	this->specular = specular;
	this->ambient = ambient;
	this->dayCycle = dayCycle;
	this->shaderProgramID = shaderProgramID;
}

void DirectionalLight::Draw() {
	int position_location = glGetUniformLocation(shaderProgramID, "LightPosition");
	glUniform3fv(position_location, 1, position.v);

	int diffuse_location = glGetUniformLocation(shaderProgramID, "Ld");
	glUniform3fv(diffuse_location, 1, diffuse.v);

	int specular_location = glGetUniformLocation(shaderProgramID, "Ls");
	glUniform3fv(specular_location, 1, specular.v);

	int ambient_location = glGetUniformLocation(shaderProgramID, "La");
	glUniform3fv(ambient_location, 1, ambient.v);
}

void DirectionalLight::Update() {
	if (dayCycle) {
		// TODO Define a nice cycle to change the diffuse ambient and specular terms with the changing of the day
		return;
	}
}