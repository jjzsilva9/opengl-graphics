#include "model.h" 

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

using namespace std;

Model::Model(const char* path, GLuint shaderProgramID) {
	this->shaderProgramID = shaderProgramID;
	loadModel(path);
}

void Model::Draw(Shader& shader) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw();
	}
}

void Model::loadModel(const char* file_name) {
		   
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n%s", file_name, aiGetErrorString());
		return;
	}
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
		
	for (unsigned int m_i = 0; m_i < node->mNumMeshes; m_i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[m_i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
		
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
		Vertex vertex;
		if (mesh->HasPositions()) {
			const aiVector3D* vp = &(mesh->mVertices[v_i]);
			vertex.Position = vec3(vp->x, vp->y, vp->z);
		}
		if (mesh->HasNormals()) {
			const aiVector3D* vn = &(mesh->mNormals[v_i]);
			vertex.Normal = vec3(vn->x, vn->y, vn->z);
		}
		if (mesh->HasTextureCoords(0)) {
			const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
			vertex.TextureCoords = vec2(vt->x, vt->y);
		}
		if (mesh->HasTangentsAndBitangents()) {
			/* You can extract tangents and bitangents here              */
			/* Note that you might need to make Assimp generate this     */
			/* data for you. Take a look at the flags that aiImportFile  */
			/* can take.                                                 */
		}
		vertices.push_back(vertex);
	}

	for (unsigned int i_i = 0; i_i < mesh->mNumFaces; i_i++) {
		aiFace face = mesh->mFaces[i_i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	return Mesh(vertices, indices, textures, shaderProgramID);
}