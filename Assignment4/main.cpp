#pragma warning(disable : 5208)

// Standard library includes
#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <math.h>

namespace std {
    using ::sqrt;
    using ::sin;
    using ::acos;
}

// Windows specific
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>

// OpenGL
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Project includes
#include "maths_funcs.h"
#include "shader.h"
#include "model.h"
#include "griffin.h"
#include "PerlinNoise.h"

#define CAMERASPEED 50.0f


typedef struct
{
	vec3 position = vec3(0.0f, 30.0f, 30.0f);
	vec3 direction = vec3(0.0f, 0.0f, -1.0f);
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
} Camera;
Camera camera;

using namespace std;

int width = 800;
int height = 600;

// Root of the Hierarchy
mat4 view = translate(identity_mat4(), vec3(0.0, 0.0, -10.0f));
mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);

float delta;
float yaw, pitch;
int lastX = width / 2;
int lastY = height / 2;
bool firstMouse = true;

Shader* shader = nullptr;
Model* terrain = nullptr;
std::vector<Griffin> griffins;

#pragma region INPUT_FUNCTIONS

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	if (key == 27) {
		glutLeaveMainLoop();
	}

	if (key == 'w') {
		camera.position += camera.direction * CAMERASPEED * delta;
	}
	if (key == 'a') {
		camera.position -= normalise(cross(camera.direction, camera.up)) * CAMERASPEED * delta;
	}
	if (key == 's') {
		camera.position -= camera.direction * CAMERASPEED * delta;
	}
	if (key == 'd') {
		camera.position += normalise(cross(camera.direction, camera.up)) * CAMERASPEED * delta;
	}
	if (key == 'e') {
		camera.position += camera.up * CAMERASPEED * delta;
	}
	if (key == 'q') {
		camera.position -= camera.up * CAMERASPEED * delta;
	}
}



void mouse(int x, int y) {
	//std::cout << "Mouse moved to: " << x << ", " << y << std::endl;
	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	float xoffset = x - lastX;
	float yoffset = lastY - y;

	lastX = x;
	lastY = y;

	yaw += xoffset * 0.1f;
	pitch += yoffset * 0.1f;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}
	vec3 viewDirection;
	viewDirection.v[0] = cos(radian(yaw)) * cos(radian(pitch));
	viewDirection.v[1] = sin(radian(pitch));
	viewDirection.v[2] = sin(radian(yaw)) * cos(radian(pitch));
	camera.direction = normalise(viewDirection);

	if ((x < width / 100) || (x > 99 * width / 100) || (y < height / 100) || (y > 99 * height / 100)) {
		glutWarpPointer(width / 2, height / 2);
		lastX = width / 2;
		lastY = height / 2;
	}
}

//void mouseEnter(int state) {
//	firstMouse = true;
//	glutWarpPointer(width / 2, height / 2);
//}

void reshape(int x, int y) {
	width = x;
	height = y;
	glViewport(0, 0, x, y);
	persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
}

#pragma endregion INPUT_FUNCTIONS

#pragma region TERRAIN

// Placeholder for procedural terrain for final deliverable
//ModelData generateTerrain(int gridSize) {
//	ModelData terrain;
//	PerlinNoise p = PerlinNoise();
//
//	for (int x = 0; x < gridSize; x++) {
//		for (int z = 0; z < gridSize; z++) {
//			terrain.mVertices.push_back(vec3(x, p.noise(x, 1, z), z));
//			terrain.mPointCount++;
//		}
//	}
//	return terrain;
//}
#pragma endregion TERRAIN


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader->use();

	//Declare your uniform variables that will be used in your shader
	int view_mat_location = glGetUniformLocation(shader->ID, "view");
	int proj_mat_location = glGetUniformLocation(shader->ID, "proj");

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

	terrain->Draw();
	
	for (unsigned int i = 0; i < griffins.size(); i++) {
		griffins[i].Draw(delta);
	}
	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	view = look_at(camera.position, camera.position + camera.direction, camera.up);
	
	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	shader = new Shader("simpleVertexShader.txt", "simpleFragmentShader.txt");
	terrain = new Model("terrain_with_rocks.obj", vec3(0, 0, 0), shader->ID);
	std::cout << "Griffin Model:" << "\n";

	GriffinFiles files{ "griffin_body_shrunk.obj", "griffin_leftwing_shrunk.obj", "griffin_rightwing_shrunk.obj" };

	griffins.push_back(Griffin(files, vec3(0, 15.0f, 0), 5.0f, 0.25f, shader->ID));
	griffins.push_back(Griffin(files, vec3(25, 30, -20), 10.0f, 0.1f, shader->ID));
	griffins.push_back(Griffin(files, vec3(-25, 22.5f, 20), 7.5f, 0.2f, shader->ID));
	griffins.push_back(Griffin(files, vec3(0, 50.0f, 0), 8.0f, 0.25f, shader->ID));
	griffins.push_back(Griffin(files, vec3(60, 28.0f, -60), 10.0f, 0.1f, shader->ID));
	griffins.push_back(Griffin(files, vec3(-70, 20.0f, 65), 9.0f, 0.2f, shader->ID));
	griffins.push_back(Griffin(files, vec3(-80, 18.0f, 80), 7.0f, 0.18f, shader->ID));
	griffins.push_back(Griffin(files, vec3(75, 17.0f, 75), 8.5f, 0.22f, shader->ID));
	griffins.push_back(Griffin(files, vec3(-90, 19.0f, 70), 10.0f, 0.15f, shader->ID));
	griffins.push_back(Griffin(files, vec3(85, 18.0f, 70), 9.5f, 0.17f, shader->ID));
	griffins.push_back(Griffin(files, vec3(50, 30.0f, -70), 11.0f, 0.16f, shader->ID));
	griffins.push_back(Griffin(files, vec3(-60, 28.0f, -55), 8.0f, 0.14f, shader->ID));
	griffins.push_back(Griffin(files, vec3(20, 36.0f, 15), 12.0f, 0.12f, shader->ID));

}



int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Project");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutSetCursor(GLUT_CURSOR_NONE);
	glutPassiveMotionFunc(mouse);
	glutReshapeFunc(reshape);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	glutFullScreen();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
