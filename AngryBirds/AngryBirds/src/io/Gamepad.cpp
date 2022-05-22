#include "Gamepad.h"

#include <iostream>
#include <fstream>

Gamepad::Gamepad(int i, const char* mappingPath) {
	id = getId(i);

	std::ifstream t(mappingPath);
	std::string mappings((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	auto mappingSuccess = glfwUpdateGamepadMappings(mappings.c_str());
	if (mappingSuccess == GLFW_TRUE) {
		std::cout << "Controller successfully mapped" << std::endl;
	}
	else {
		std::cout << "Controller not mapped" << std::endl;
	}

	update();
}

void Gamepad::update() {
	present = glfwJoystickIsGamepad(id);

	if (present) {
		name = glfwGetGamepadName(id);
		glfwGetGamepadState(id, &state);
	}
}

float Gamepad::axesState(int axis) {
	if (present) {
		return state.axes[axis];
	}
	return -1;
}

unsigned char Gamepad::buttonState(int button) {
	if (present) {
		return state.buttons[button];
	}
	return GLFW_RELEASE;
}

bool Gamepad::isPresent() {
	return present;
}

const char* Gamepad::getName() {
	return name;
}

int Gamepad::getId(int i) {
	return GLFW_JOYSTICK_1 + i;
}

