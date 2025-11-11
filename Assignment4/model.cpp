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

Model::Model(const char* path, vec3 position, GLuint shaderProgramID) {
	this->shaderProgramID = shaderProgramID;
	this->model = identity_mat4();
	this->model.m[12] = position.v[0];
	this->model.m[13] = position.v[1];
	this->model.m[14] = position.v[2];
	loadModel(path);
}

void Model::Draw() {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(model);
	}
}

void Model::translate(vec3 offset) {
	model = ::translate(model, offset);
}

void Model::rotate(vec3 offset) {
	model = rotate_x_deg(model, offset.v[0]);
	model = rotate_y_deg(model, offset.v[1]);
	model = rotate_z_deg(model, offset.v[2]);
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

	aiReleaseImport(scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
		
	for (unsigned int m_i = 0; m_i < node->mNumMeshes; m_i++) {
		std::cout << "Mesh number 1" << std::endl;
		aiMesh* mesh = scene->mMeshes[node->mMeshes[m_i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		std::cout << "Node" << node->mName.C_Str() << std::endl;
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
	std::cout << "Faces done" << "\n";

	return Mesh(vertices, indices, textures, shaderProgramID);
}