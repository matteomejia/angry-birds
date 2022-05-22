#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <GLFW/glfw3.h>

class Gamepad {
public:
	Gamepad(int i, const char *mappingPath);

	void update();

	float axesState(int axis);

	unsigned char buttonState(int button);

	bool isPresent();

	const char* getName();

	static int getId(int i);

private:

	int present;

	int id;

	const char* name;

	GLFWgamepadstate state;
};

#endif