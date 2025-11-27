#include "mesh.h"

// Standard library
#include <string>
#include <vector>
#include <math.h>

namespace std {
    using ::sqrt;
    using ::sin;
    using ::acos;
}

// OpenGL
#include <GL/glew.h>
#include <GL/freeglut.h>

// Project includes
#include "shader.h"

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

using namespace std;

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Shader* shader) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
	this->shader = shader;
	this->shaderProgramID = shader->ID;
    setupMesh();
}

void Mesh::Draw(mat4 model) {

	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int normalNr = 1;
	unsigned int heightNr = 1;
	for (unsigned int i = 0; i < textures.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		string number;
		string name = textures[i].type;
		if (name == "texture_diffuse") {
			number = std::to_string(diffuseNr++);
		}
		else if (name == "texture_specular") {
			number = std::to_string(specularNr++);
		}
		else if (name == "texture_normal") {
			number = std::to_string(normalNr++);
		}
		else if (name == "texture_height") {
			number = std::to_string(heightNr++);
		}
		shader->setInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}

	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
}
    
void Mesh::setupMesh() {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	GLuint loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	GLuint loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	GLuint loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	
	glEnableVertexAttribArray(loc1);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);

	glEnableVertexAttribArray(loc2);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

	glEnableVertexAttribArray (loc3);
	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));

	glBindVertexArray(0);
	std::cout << "Mesh setup" << "\n";
}