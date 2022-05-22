#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/rendering/Shader.h"
#include "graphics/rendering/Texture.h"

#include "io/Keyboard.h"
#include "io/Mouse.h"
#include "io/Gamepad.h"
#include "io/Camera.h"

#include "graphics/models/Cube.hpp"
#include "graphics/models/Lamp.hpp"

#include "Screen.h"

void processInput(double dt);

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

Screen screen;

std::string Shader::defaultDirectory = "resources/shaders";

float mixVal = 0.5f;

glm::mat4 mouseTranform = glm::mat4(1.0f);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
	// Inicializacion de glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	if (!screen.init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Inicializacion de glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	screen.setParameters();

	glEnable(GL_DEPTH_TEST);

	Shader shader(false, "object.vert", "object.frag");
	Shader lampShader(false, "object.vert", "lamp.frag");

	Cube cube(Material::gold, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.75f));
	cube.init();

	Lamp lamp(glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(-1.0f, -0.5f, -0.5f), glm::vec3(0.25f));
	lamp.init();

	while (!screen.shouldClose()) {
		double currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		processInput(deltaTime);

		screen.update();

		shader.activate();

		shader.set3Float("light.position", lamp.pos);
		shader.set3Float("viewPos", camera.cameraPos);

		shader.set3Float("light.ambient", lamp.ambient);
		shader.set3Float("light.diffuse", lamp.diffuse);
		shader.set3Float("light.specular", lamp.specular);

		// transformation for screen
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = camera.getViewMatrix();
		projection = glm::perspective(glm::radians(camera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		cube.render(shader);

		lampShader.activate();
		lampShader.setMat4("view", view);
		lampShader.setMat4("projection", projection);
		lamp.render(lampShader);

		screen.newFrame();
	}

	cube.cleanup();
	lamp.cleanup();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(double dt) {
	if (Keyboard::key(GLFW_KEY_ESCAPE))
		screen.setShouldClose(true);

	// change mixVal
	if (Keyboard::key(GLFW_KEY_UP)) {
		mixVal += 0.5;
		if (mixVal > 1) {
			mixVal = 1.0f;
		}
	}


	if (Keyboard::key(GLFW_KEY_DOWN)) {
		mixVal -= 0.5;
		if (mixVal < 0) {
			mixVal = 0.0f;
		}
	}

	if (Keyboard::key(GLFW_KEY_W)) {
		camera.updateCameraPos(CameraDirection::FORWARD, dt);
	}

	if (Keyboard::key(GLFW_KEY_S)) {
		camera.updateCameraPos(CameraDirection::BACKWARD, dt);
	}

	if (Keyboard::key(GLFW_KEY_A)) {
		camera.updateCameraPos(CameraDirection::LEFT, dt);
	}

	if (Keyboard::key(GLFW_KEY_D)) {
		camera.updateCameraPos(CameraDirection::RIGHT, dt);
	}

	if (Keyboard::key(GLFW_KEY_SPACE)) {
		camera.updateCameraPos(CameraDirection::UP, dt);
	}

	if (Keyboard::key(GLFW_KEY_LEFT_CONTROL)) {
		camera.updateCameraPos(CameraDirection::DOWN, dt);
	}

	double dx = Mouse::getDX(), dy = Mouse::getDY();

	if (dx != 0 || dy != 0) {
		camera.updateCameraDirection(dx, dy);
	}

	double scrollDy = Mouse::getScrollDY();
	if (scrollDy != 0) {
		camera.updateCameraZoom(scrollDy);
	}
}