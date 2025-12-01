#pragma once

// Standard library
#include <string>
#include <vector>
#include <math.h>

// Project includes - needed for definitions
#include "maths_funcs.h"
#include "shader.h"
#include "fog.h"
#include "directionallight.h"

Fog::Fog(vec4 color, float maxdist, float mindist, FogFactor factor, GLuint shaderProgramID, DirectionalLight* lightSource, bool enabled) {
	this->shaderProgramID = shaderProgramID;
	this->color = color;
	this->maxdist = maxdist;
	this->mindist = mindist;
	this->factor = factor;
	this->lightSource = lightSource;
	this->enabled = enabled;
	this->Update();
}

void Fog::Update() {
	int color_location = glGetUniformLocation(shaderProgramID, "fog_color");
	glUniform4fv(color_location, 1, color.v);

	int maxdist_location = glGetUniformLocation(shaderProgramID, "fog_maxdist");
	glUniform1f(maxdist_location, maxdist);

	int mindist_location = glGetUniformLocation(shaderProgramID, "fog_mindist");
	glUniform1f(mindist_location, mindist);

	int factor_location = glGetUniformLocation(shaderProgramID, "fog_factor");
	glUniform1i(factor_location, factor);

	int enabled_location = glGetUniformLocation(shaderProgramID, "fog_enabled");
	glUniform1i(enabled_location, enabled);
}

void Fog::Draw() {

	color = getFogColor(lightSource->timeOfDay);

	int color_location = glGetUniformLocation(shaderProgramID, "fog_color");
	glUniform4fv(color_location, 1, color.v);
}

vec4 Fog::getFogColor(float time) {
	// Night fog - dark blue/purple
	vec4 nightFog = vec4(0.05f, 0.08f, 0.15f, 1.0f);

	// Dawn fog - pink/orange morning mist
	vec4 dawnFog = vec4(0.9f, 0.6f, 0.5f, 1.0f);

	// Day fog - light gray/white
	vec4 dayFog = vec4(0.7f, 0.75f, 0.8f, 1.0f);

	// Dusk fog - orange/red evening haze
	vec4 duskFog = vec4(0.85f, 0.5f, 0.3f, 1.0f);

	if (time < 0.25f) {
		// Midnight to sunrise (0.0 - 0.25)
		float t = time / 0.25f;
		return vec4(
			nightFog.v[0] + t * (dawnFog.v[0] - nightFog.v[0]),
			nightFog.v[1] + t * (dawnFog.v[1] - nightFog.v[1]),
			nightFog.v[2] + t * (dawnFog.v[2] - nightFog.v[2]),
			1.0f
		);
	}
	else if (time < 0.5f) {
		// Sunrise to noon (0.25 - 0.5)
		float t = (time - 0.25f) / 0.25f;
		return vec4(
			dawnFog.v[0] + t * (dayFog.v[0] - dawnFog.v[0]),
			dawnFog.v[1] + t * (dayFog.v[1] - dawnFog.v[1]),
			dawnFog.v[2] + t * (dayFog.v[2] - dawnFog.v[2]),
			1.0f
		);
	}
	else if (time < 0.75f) {
		// Noon to sunset (0.5 - 0.75)
		float t = (time - 0.5f) / 0.25f;
		return vec4(
			dayFog.v[0] + t * (duskFog.v[0] - dayFog.v[0]),
			dayFog.v[1] + t * (duskFog.v[1] - dayFog.v[1]),
			dayFog.v[2] + t * (duskFog.v[2] - dayFog.v[2]),
			1.0f
		);
	}
	else {
		// Sunset to midnight (0.75 - 1.0)
		float t = (time - 0.75f) / 0.25f;
		return vec4(
			duskFog.v[0] + t * (nightFog.v[0] - duskFog.v[0]),
			duskFog.v[1] + t * (nightFog.v[1] - duskFog.v[1]),
			duskFog.v[2] + t * (nightFog.v[2] - duskFog.v[2]),
			1.0f
		);
	}
}