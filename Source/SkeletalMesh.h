#ifndef SKELETAL_MESH_H
#define SKELETAL_MESH_H

#include <string>
#include <vector>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Texture.h"


class SkeletalMesh {
public:
	std::vector <SkeletalVertex> vertices;
	std::vector <GLuint> indices;
	std::vector <Texture> textures; // should be vector of pointers, but won't optimize much.
	glm::mat4 model; // sends mesh to default model position (same as in blender)

	SkeletalVAO* m_VAO;
	SkeletalVBO* m_VBO;
	EBO* m_EBO;

	SkeletalMesh(std::vector <SkeletalVertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures, glm::mat4& transformation);
	~SkeletalMesh();

	void Draw
	(
		Shader& shader,
		Camera& camera,
		glm::vec3& translation,
		glm::quat& rotation,
		glm::vec3& scale
	);
};

#endif