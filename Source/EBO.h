#ifndef EBO_H
#define EBO_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

class EBO
{
public:
	// ID reference of Elements Buffer Object
	GLuint ID;
	EBO() {};
/*
	EBO(EBO&& other) {
        EBO::ID = std::move(other.ID);
	}
	EBO& operator=(EBO&& other){
        this->ID = std::move(other.ID);
		return *this;
	}
*/
	// Constructor that generates a Elements Buffer Object and links it to indices
	EBO(std::vector<GLuint>& indices);

	// Binds the EBO
	void Bind();
	// Unbinds the EBO
	void Unbind();
	// Deletes the EBO
	void Delete();
};

#endif
