#include "Gamepad.h"

Gamepad::Gamepad(int i) {
	id = getId(i);
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

