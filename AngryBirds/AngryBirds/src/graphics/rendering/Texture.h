#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

/*
	class to represent texture
*/

class Texture {
public:
	/*
		constructor
	*/

	// initialize with name
	Texture();

	// initialize with image path and type
	Texture(const char* path, const char* name, bool defaultParams = true);

	// generate texture id
	void generate();
	void load(bool flip = true);

	void setFilters(GLenum all);
	void setFilters(GLenum mag, GLenum min);

	void setWrap(GLenum all);
	void setWrap(GLenum s, GLenum t);

	void bind();

	/*
		texture object values
	*/

	// texture id
	int id;
	unsigned int tex;
	// name
	const char* name;

private:
	static int currentId;

	const char* path;
	int width;
	int height;
	int nChannels;
};

#endif