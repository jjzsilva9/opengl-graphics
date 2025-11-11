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

Griffin::Griffin(const char* body_path, const char* left_wing_path, const char* right_wing_path, GLuint shaderProgramID) {
    models.push_back(Model(body_path, vec3(0, 0, 0), shaderProgramID));
    models.push_back(Model(left_wing_path, vec3(0, 0, 0), shaderProgramID));
    models.push_back(Model(right_wing_path, vec3(0, 0, 0), shaderProgramID));

    animationTime = 0.0f;
}

void Griffin::Draw(float deltaTime) {

    animationTime += deltaTime;

    float angle = animationTime * 1 / 2 * 3.14f;
    vec3 flightPos = vec3(cos(angle), 0, sin(angle));
    float heading = angle;
    float wingAngle = 100.0f + 50.0f * sin(angle);

    vec3 base_orientation = vec3(0, 0, 0);
    vec3 base_translation = flightPos;
    mat4 local1 = identity_mat4();
    local1 = rotate_x_deg(local1, base_orientation.v[0]);
    local1 = rotate_y_deg(local1, base_orientation.v[1]);
    local1 = rotate_z_deg(local1, base_orientation.v[2]);
    local1 = translate(local1, base_translation);

    models[0].model = local1;
    models[0].Draw();

    vec3 joint1_orientation = vec3(0, 0, wingAngle);
    vec3 joint1_translation = vec3(0, 0, 0);
    mat4 local2 = identity_mat4();
    local2 = rotate_x_deg(local2, joint1_orientation.v[0]);
    local2 = rotate_y_deg(local2, joint1_orientation.v[1]);
    local2 = rotate_z_deg(local2, joint1_orientation.v[2]);
    local2 = translate(local2, joint1_translation);

    models[1].model = local1 * local2;
    models[1].Draw();

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