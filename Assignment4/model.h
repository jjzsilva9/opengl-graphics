#pragma once

// Standard library
#include <string>
#include <vector>
#include <math.h>

// Forward declarations
class Shader;
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

// Only needed for declarations
typedef unsigned int GLuint;

// Project includes - needed for definitions
#include "maths_funcs.h"
#include "mesh.h"  // Added for Texture struct

class Model {
public:
	mat4 model;

	Model(const char* path, vec3 position, GLuint shaderProgramID);
	void Draw();
	void translate(vec3 offset);
	void rotate(vec3 offset);

private:
	std::vector<Mesh> meshes;
	std::string directory;
	GLuint shaderProgramID;
	
	void loadModel(const char* file_name);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};