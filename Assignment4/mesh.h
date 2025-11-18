#pragma once

// Standard library - only what's needed for declarations
#include <string>
#include <vector>

// Forward declare GL type
typedef unsigned int GLuint;

// Only needed for vec2/vec3
#include "maths_funcs.h"
#include "shader.h"

struct Vertex {
    vec3 Position;
    vec3 Normal;
    vec2 TextureCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
	aiString path;
};

class Mesh {
public:
	std::vector<Vertex>       vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture>      textures;
	
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Shader* shader);

	void Draw(mat4 model);
private:
	unsigned int VAO, VBO, EBO;
	GLuint shaderProgramID;
	Shader* shader;

	void setupMesh();
};