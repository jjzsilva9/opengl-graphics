#pragma warning(disable : 5208)

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

struct Vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TextureCoords;
};

struct Texture {
	unsigned int id;
	std::string type;
};

class Mesh {
    public:
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture>      textures;

        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Shader shader) {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;
			this->shader = shader;
            setupMesh();
        }

        void Draw(Shader& shader) {

        }
    private:
        unsigned int VAO, VBO, EBO;
		Shader shader;

        void setupMesh() {

			GLuint loc1 = glGetAttribLocation(shader->shaderProgramID, "vertex_position");
			GLuint loc2 = glGetAttribLocation(shader->shaderProgramID, "vertex_normal");
			GLuint loc3 = glGetAttribLocation(shader->shaderProgramID, "vertex_texture");

			// Vertices
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &VAO);

			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

			glEnableVertexAttribArray(loc1);
			glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);

			glEnableVertexAttribArray(loc2);
			glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

			glEnableVertexAttribArray (loc3);
			glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));
        }
};