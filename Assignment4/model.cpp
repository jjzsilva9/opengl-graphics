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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

std::vector<Texture> Model::textures_loaded;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

Model::Model(const char* path, vec3 position, Shader* shader) {
	this->shaderProgramID = shaderProgramID;
	this->model = identity_mat4();
	this->model.m[12] = position.v[0];
	this->model.m[13] = position.v[1];
	this->model.m[14] = position.v[2];
	this->shader = shader;
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
		aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n%s", file_name, aiGetErrorString());
		return;
	}

	directory = std::string(file_name).substr(0, std::string(file_name).find_last_of('\\/'));
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
			vertex.TextureCoords = vec2(vt->x, -vt->y);
		}
		else {
			vertex.TextureCoords = vec2(0.0f, 0.0f);
		}

		if (mesh->HasTangentsAndBitangents()) {
			const aiVector3D* vta = &(mesh->mTangents[v_i]);
			vertex.Tangent = vec3(vta->x, vta->y, vta->z);

			const aiVector3D* vb = &(mesh->mBitangents[v_i]);
			vertex.Bitangent = vec3(vb->x, vb->y, vb->z);
		}
		vertices.push_back(vertex);
	}

	for (unsigned int i_i = 0; i_i < mesh->mNumFaces; i_i++) {
		
		aiFace face = mesh->mFaces[i_i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}
	std::cout << "Faces done" << "\n";

	return Mesh(vertices, indices, textures, shader);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++) {
			std::cout << textures_loaded[j].path.C_Str() << ", " << str.C_Str() << std::endl;
			if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0) {
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			Texture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typeName;
			texture.path = str;
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma) {
	string filename = string(path);
	std::replace(filename.begin(), filename.end(), '\\', '/');
	//filename = directory + '/' + filename;
	std::cout << filename << std::endl;
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << filename << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}