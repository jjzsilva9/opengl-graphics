#pragma warning(disable : 5208)

// Windows includes (For Time, IO, etc.)
#define NOMINMAX
#include <limits>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"
#include "PerlinNoise.h"



/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME "snowymountain.obj"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#define CAMERASPEED 10.0f
#pragma region SimpleTypes

typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

typedef struct
{
	vec3 position = vec3(0.0f, 0.0f, 0.0f);
	vec3 direction = vec3(0.0f, 0.0f, -1.0f);
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
} Camera;
Camera camera;

using namespace std;
GLuint shaderProgramID;

ModelData mesh_data;
unsigned int mesh_vao = 0;
ModelData terrain_data;
unsigned int terrain_vao = 0;
int width = 800;
int height = 600;

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;

// Root of the Hierarchy
mat4 view = translate(identity_mat4(), vec3(0.0, 0.0, -10.0f));
mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
mat4 model = identity_mat4();

float delta;
float yaw, pitch;
int lastX = width / 2;
int lastY = height / 2;
bool firstMouse = true;

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name, 
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	); 

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n%s", file_name, aiGetErrorString());
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING


#pragma region INPUT_FUNCTIONS

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	if (key == 'x') {
		//Translate the base, etc.
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
	if (key == 'm') {
		model = scale(model, vec3(1.5f, 1.5f, 1.5f));
	}
	if (key == 'n') {
		model = scale(model, vec3(0.75f, 0.75f, 0.75f));
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
	persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
}

#pragma endregion INPUT_FUNCTIONS

#pragma region TERRAIN


ModelData generateTerrain(int gridSize) {
	ModelData terrain;
	PerlinNoise p = PerlinNoise();

	for (int x = 0; x < gridSize; x++) {
		for (int z = 0; z < gridSize; z++) {
			terrain.mVertices.push_back(vec3(x, p.noise(x, 1, z), z));
			terrain.mPointCount++;
		}
	}
	return terrain;
}
#pragma endregion TERRAIN


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);


	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

	//glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

	//glDrawArrays(GL_TRIANGLES, 0, terrain_data.mPointCount);

	// Set up the child matrix
	//mat4 modelChild = identity_mat4();
	//modelChild = rotate_z_deg(modelChild, 180);
	//modelChild = rotate_y_deg(modelChild, rotate_y);
	//modelChild = translate(modelChild, vec3(0.0f, 1.9f, 0.0f));

	//// Apply the root matrix to the child matrix
	//modelChild = model * modelChild;

	//// Update the appropriate uniform and draw the mesh again
	//glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelChild.m);
	//glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

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
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load mesh into a vertex buffer array
	mesh_data = load_mesh(MESH_NAME);
	generateObjectBufferMesh(mesh_data);

	/*terrain_data = generateTerrain(1000);
	generateObjectBufferMesh(terrain_data);*/

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
	//glutSetCursor(GLUT_CURSOR_NONE);
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
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
