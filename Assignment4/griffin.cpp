#include "griffin.h"

// Standard library
#include <string>
#include <stdio.h>
#include <vector>
#include <math.h>

namespace std {
    using ::sqrt;
    using ::sin;
    using ::acos;
}

// Assimp includes
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Project includes
#include "maths_funcs.h"
#include "shader.h"
#include "mesh.h"
#include "model.h" 

using namespace std;

Griffin::Griffin(GriffinFiles file_paths, vec3 position, float flightRadius, float flightSpeed, GLuint shaderProgramID) {
    models.push_back(Model(file_paths.body_path, position, shaderProgramID));
    models.push_back(Model(file_paths.left_wing_path, position, shaderProgramID));
    models.push_back(Model(file_paths.right_wing_path, position, shaderProgramID));
    spawnPosition = position;
    animationTime = 0.0f;
    this->flightRadius = flightRadius;
    this->flightSpeed = flightSpeed;
}

void Griffin::Draw(float deltaTime) {

    animationTime += deltaTime;

    float angle = animationTime * flightSpeed * 3.14f;;

    // Slight varying offset to simulate wind
    vec3 windOffset = vec3(0.2f * cos(angle), sin(angle * 1.5f) * 0.025f, 0.2f * sin(angle));

    // Circular flight path
    vec3 flightPath = vec3(cos(angle * 0.2f) * flightRadius, 0, sin(angle * 0.2f) * flightRadius);

    // Heading to maintain rotation through the flight path
    float heading = angle * 0.2f * 180.0f / 3.14f;

    // Cyclical wing animation
    float wingAngle = 100.0f + 50.0f * sin(angle * 5.0f);

    // Setting position and rotation of the body
    vec3 base_orientation = vec3(0, -heading, 20.0f);
    vec3 base_translation = flightPath + windOffset + spawnPosition;

    mat4 local1 = identity_mat4();
    local1 = rotate_x_deg(local1, base_orientation.v[0]);
    local1 = rotate_z_deg(local1, base_orientation.v[2]);
    local1 = rotate_y_deg(local1, base_orientation.v[1]);
    local1 = translate(local1, base_translation);

    models[0].model = local1;
    models[0].Draw();

    // Setting position and rotation of left wing relative to the body

    vec3 joint1_orientation = vec3(0, 0, wingAngle);
    vec3 joint1_translation = vec3(0, 0, 0);
    mat4 local2 = identity_mat4();
    local2 = rotate_x_deg(local2, joint1_orientation.v[0]);
    local2 = rotate_y_deg(local2, joint1_orientation.v[1]);
    local2 = rotate_z_deg(local2, joint1_orientation.v[2]);
    local2 = translate(local2, joint1_translation);

    models[1].model = local1 * local2;
    models[1].Draw();

    // Setting position and rotation of right wing relative to the body

    vec3 joint2_orientation = vec3(0, 0, -wingAngle);
    vec3 joint2_translation = vec3(0, 0, 0);
    mat4 local3 = identity_mat4();
    local3 = rotate_x_deg(local3, joint2_orientation.v[0]);
    local3 = rotate_y_deg(local3, joint2_orientation.v[1]);
    local3 = rotate_z_deg(local3, joint2_orientation.v[2]);
    local3 = translate(local3, joint2_translation);

    models[2].model = local1 * local3;
    models[2].Draw();
}