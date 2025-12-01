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
#include "directionallight.h"
#include "fog.h"
#include "skybox.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

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
Shader* skyboxShader = nullptr;
Model* terrain = nullptr;
std::vector<Griffin> griffins;
DirectionalLight* lightSource = nullptr;
Fog* fog = nullptr;
Skybox* skybox = nullptr;

bool showGUI = false;
float normalMapIntensity = 1.0f;

#pragma region INPUT_FUNCTIONS

void keypress(unsigned char key, int x, int y) {
	ImGuiIO& io = ImGui::GetIO();

	if (key == 'g') {
		showGUI = !showGUI;
		if (showGUI) {
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
		}
		else {
			glutSetCursor(GLUT_CURSOR_NONE);
		}
		return;
	}
	io.AddInputCharacter(key);
	if (io.WantCaptureKeyboard) {
		return;
	}
	// ESC Key to quit
	if (key == 27) {
		glutLeaveMainLoop();
	}

	// Forward/Backward and sideways movement
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

	// Up/Down Movement
	if (key == 'e') {
		camera.position += camera.up * CAMERASPEED * delta;
	}
	if (key == 'q') {
		camera.position -= camera.up * CAMERASPEED * delta;
	}
}



void mouse(int x, int y) {
	ImGuiIO& io = ImGui::GetIO();

	
	io.MousePos = ImVec2((float)x, (float)y);
	if (io.WantCaptureMouse) {
		return;
	}
	if (showGUI) {
		return;
	}
	// If the mouse has just entered the window
	// Set lastX and lastY to the current x and y
	if (firstMouse) {
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	// Calculate difference from last frame
	float xoffset = x - lastX;
	float yoffset = lastY - y;

	lastX = x;
	lastY = y;

	// Convert to yaw and pitch
	yaw += xoffset * 0.1f;
	pitch += yoffset * 0.1f;

	// Clip max and min pitch
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	// Calculate viewing direction
	vec3 viewDirection;
	viewDirection.v[0] = cos(radian(yaw)) * cos(radian(pitch));
	viewDirection.v[1] = sin(radian(pitch));
	viewDirection.v[2] = sin(radian(yaw)) * cos(radian(pitch));
	camera.direction = normalise(viewDirection);

	// If the cursor is near the edge of the screen, warp it to the centre
	if ((x < width / 100) || (x > 99 * width / 100) || (y < height / 100) || (y > 99 * height / 100)) {
		glutWarpPointer(width / 2, height / 2);
		lastX = width / 2;
		lastY = height / 2;
	}
}

void mouseClick(int button, int state, int x, int y) {
	ImGuiIO& io = ImGui::GetIO();

	if (button == GLUT_LEFT_BUTTON) {
		io.MouseDown[0] = (state == GLUT_DOWN);
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		io.MouseDown[1] = (state == GLUT_DOWN);
	}
	else if (button == GLUT_MIDDLE_BUTTON) {
		io.MouseDown[2] = (state == GLUT_DOWN);
	}
}

void mouseDrag(int x, int y) {
	ImGuiIO& io = ImGui::GetIO();

	io.MousePos = ImVec2((float)x, (float)y);

	if (io.WantCaptureMouse || showGUI) {
		return;
	}
}

void reshape(int x, int y) {
	width = x;
	height = y;
	glViewport(0, 0, x, y);
	persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)width, (float)height);
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

void renderGUI() {
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)width, (float)height);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGLUT_NewFrame();
	ImGui::NewFrame();

	if (showGUI) {
		float windowWidth = 450.0f;
		ImGui::SetNextWindowSize(ImVec2(windowWidth, 650), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(width - windowWidth - 10, 10), ImGuiCond_FirstUseEver);
		ImGui::Begin("Scene Settings", &showGUI);

		if (ImGui::CollapsingHeader("Time of Day", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Day/Night Cycle##Light", &lightSource->dayCycle);
			if (!lightSource->dayCycle) {
				ImGui::DragFloat("Time", &lightSource->timeOfDay, 0.001f, 0.0f, 1.0f);
			}
		}

		if (ImGui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_DefaultOpen)) {
			bool changed = false;
			changed |= ImGui::Checkbox("Fog Enabled", &fog->enabled);
			changed |= ImGui::DragFloat("Max Dist", &fog->maxdist, 1.0f, 50.0f, 500.0f);
			changed |= ImGui::DragFloat("Min Dist", &fog->mindist, 1.0f, 0.0f, fog->maxdist - 5.0f);
			int type = (int)fog->factor;
			if (ImGui::Combo("Type", &type, "Linear\0Exponential\0Exp Squared\0")) {
				fog->factor = (FogFactor)type;
				changed = true;
			}
			if (changed) fog->Update();
		}

		if (ImGui::CollapsingHeader("Normal Map")) {
			ImGui::DragFloat("Intensity", &normalMapIntensity, 1.0f, 0.0f, 1000.0f);
		}

		if (ImGui::CollapsingHeader("Materials")) {
			if (ImGui::TreeNode("Terrain")) {
				bool changed = false;
				changed |= ImGui::ColorEdit3("Kd##T", terrain->meshes[0].textures[0].material.Kd.v);
				changed |= ImGui::ColorEdit3("Ks##T", terrain->meshes[0].textures[0].material.Ks.v);
				changed |= ImGui::ColorEdit3("Ka##T", terrain->meshes[0].textures[0].material.Ka.v);
				changed |= ImGui::DragFloat("Ns##T", &terrain->meshes[0].textures[0].material.Ns, 1.0f, 1.0f, 1000.0f);
				ImGui::TreePop();

				//if (changed) terrain->changeMeshMaterials();
			}
		}

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void display() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);
	skyboxShader->use();
	
	mat4 skyboxView = view;
	skyboxView.m[12] = skyboxView.m[13] = skyboxView.m[14] = 0.0f;
	
	int view_mat_location = glGetUniformLocation(skyboxShader->ID, "view");
	int proj_mat_location = glGetUniformLocation(skyboxShader->ID, "projection");
	int skybox_location = glGetUniformLocation(skyboxShader->ID, "skybox");
	int time_location = glGetUniformLocation(skyboxShader->ID, "timeOfDay");

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, skyboxView.m);
	glUniform1i(skybox_location, 0);
	glUniform1f(time_location, lightSource->timeOfDay);

	glBindVertexArray(skybox->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->textureID);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDepthFunc(GL_LESS);
	shader->use();

	view_mat_location = glGetUniformLocation(shader->ID, "view");
	proj_mat_location = glGetUniformLocation(shader->ID, "proj");

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniform1f(glGetUniformLocation(shader->ID, "normalMapIntensity"), normalMapIntensity);

	lightSource->Draw(delta);
	fog->Draw();

	terrain->Draw();
	
	for (unsigned int i = 0; i < griffins.size(); i++) {
		griffins[i].Draw(delta);
	}

	renderGUI();
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
	skyboxShader = new Shader("skyboxVertexShader.txt", "skyboxFragmentShader.txt");
	
	terrain = new Model("terrain_with_textures.obj", vec3(0, 0, 0), shader);
	std::cout << "Griffin Model:" << "\n";

	GriffinFiles files{ "griffin_body_shrunk_with_textures.obj", "griffin_leftwing_shrunk_with_textures.obj", "griffin_rightwing_shrunk_with_textures.obj" };

	griffins.push_back(Griffin(files, vec3(0, 15.0f, 0), 5.0f, 0.25f, shader));
	griffins.push_back(Griffin(files, vec3(25, 30, -20), 10.0f, 0.1f, shader));
	griffins.push_back(Griffin(files, vec3(-25, 22.5f, 20), 7.5f, 0.2f, shader));
	griffins.push_back(Griffin(files, vec3(0, 50.0f, 0), 8.0f, 0.25f, shader));
	griffins.push_back(Griffin(files, vec3(60, 28.0f, -60), 10.0f, 0.1f, shader));
	griffins.push_back(Griffin(files, vec3(-70, 20.0f, 65), 9.0f, 0.2f, shader));
	griffins.push_back(Griffin(files, vec3(-80, 18.0f, 80), 7.0f, 0.18f, shader));
	griffins.push_back(Griffin(files, vec3(50, 30.0f, -70), 11.0f, 0.16f, shader));
	griffins.push_back(Griffin(files, vec3(-60, 28.0f, -55), 8.0f, 0.14f, shader));

	lightSource = new DirectionalLight(vec4(10.0, 10.0, 4.0, 1.0), vec3(0.7, 0.7, 0.7), vec3(1.0, 1.0, 1.0), vec3(0.5, 0.5, 0.5), shader->ID, true);
	fog = new Fog(vec4(0.7f, 0.75f, 0.8f, 0.3f), 350.0f, 75.0f, EXPONENTIAL_SQUARED, shader->ID, lightSource, false);

	std::vector<std::string> faces = { 
		"textures/cubemaps/px.png",  // Right  (+X)
		"textures/cubemaps/nx.png",// Left   (-X)
		"textures/cubemaps/py.png",  // Top    (+Y)
		"textures/cubemaps/ny.png",  // Bottom (-Y)
		"textures/cubemaps/pz.png",  // Front  (+Z)
		"textures/cubemaps/nz.png"   // Back   (-Z)
	};
	skybox = new Skybox(faces);
}

void cleanup() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGLUT_Shutdown();
	ImGui::DestroyContext();

	delete shader;
	delete terrain;
	delete lightSource;
	delete fog;
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
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseDrag);
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

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGLUT_Init();
	ImGui_ImplOpenGL3_Init("#version 330");

	// Set up your objects and shaders
	init();
	atexit(cleanup);
	glutFullScreen();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}

