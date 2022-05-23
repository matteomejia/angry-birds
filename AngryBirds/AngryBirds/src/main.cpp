#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <string>
#include <vector>
#include <stack>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics/rendering/Shader.h"
#include "graphics/rendering/Texture.h"
#include "graphics/rendering/Light.h"

#include "graphics/objects/Model.h"

#include "io/Keyboard.h"
#include "io/Mouse.h"
#include "io/Gamepad.h"
#include "io/Camera.h"

#include "graphics/models/Cube.hpp"
#include "graphics/models/Lamp.hpp"
#include "graphics/models/Gun.hpp"
#include "graphics/models/Sphere.hpp"

#include "Screen.h"

void processInput(double dt);

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

Screen screen;

//Gamepad mainG(0, "resources/game/data/gamecontroller.db");
Camera Camera::defaultCamera(glm::vec3(0.0f, 0.0f, 0.0f));

float dt = 0.0f;
float lastFrame = 0.0f;

bool flashLightOn = true;

SphereArray launchObjects;

std::string Shader::defaultDirectory = "resources/shaders";

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

	launchObjects.init();

	DirLight dirLight = { glm::vec3(-0.2f, -1.0f, -0.3f),
		glm::vec4(0.1f, 0.1f, 0.1f, 1.0f),
		glm::vec4(0.4f, 0.4f, 0.4f, 1.0f),
		glm::vec4(0.75f, 0.75f, 0.75f, 1.0f)
	};

	glm::vec3 pointLightPositions[] = {
			glm::vec3(0.7f,  0.2f,  2.0f),
			glm::vec3(2.3f, -3.3f, -4.0f),
			glm::vec3(-4.0f,  2.0f, -12.0f),
			glm::vec3(0.0f,  0.0f, -3.0f)
	};

	glm::vec4 ambient = glm::vec4(0.05f, 0.05f, 0.05f, 1.0f);
	glm::vec4 diffuse = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
	glm::vec4 specular = glm::vec4(1.0f);

	float k0 = 1.0f;
	float k1 = 0.09f;
	float k2 = 0.032f;

	/*Lamp lamps[4];
	for (unsigned int i = 0; i < 4; i++) {
		lamps[i] = Lamp(glm::vec3(1.0f),
			ambient, diffuse, specular,
			k0, k1, k2,
			pointLightPositions[i], glm::vec3(0.25f));
		lamps[i].init();
	}*/

	LampArray lamps;
	lamps.init();
	for (unsigned int i = 0; i < 4; i++) {
		lamps.lightInstances.push_back({
			pointLightPositions[i], k0, k1, k2, ambient, diffuse, specular });
	}

	SpotLight s = {
		Camera::defaultCamera.cameraPos, Camera::defaultCamera.cameraFront,
		glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(20.0f)),
		1.0f, 0.07f, 0.032f,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
	};

	while (!screen.shouldClose()) {
		double currentTime = glfwGetTime();
		dt = currentTime - lastFrame;
		lastFrame = currentTime;

		processInput(dt);

		screen.update();

		shader.activate();
		shader.set3Float("viewPos", Camera::defaultCamera.cameraPos);

		dirLight.direction = glm::vec3(
			glm::rotate(glm::mat4(1.0f), glm::radians(0.5f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(dirLight.direction, 1.0f)
		);
		dirLight.render(shader);

		for (unsigned int i = 0; i < 4; i++) {
			lamps.lightInstances[i].render(shader, i);
		}

		shader.setInt("noPointLights", 4);

		if (flashLightOn) {
			s.position = Camera::defaultCamera.cameraPos;
			s.direction = Camera::defaultCamera.cameraFront;
			s.render(shader, 0);
			shader.setInt("noSpotLights", 1);
		}
		else {
			shader.setInt("noSpotLights", 0);
		}

		// transformation for screen
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		view = Camera::defaultCamera.getViewMatrix();
		projection = glm::perspective(glm::radians(Camera::defaultCamera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		std::stack<int> removeObjects;
		for (int i = 0; i < launchObjects.instances.size(); i++) {
			if (glm::length(Camera::defaultCamera.cameraPos - launchObjects.instances[i].pos) > 50.0f) {
				removeObjects.push(i);
				continue;
			}
		}

		for (int i = 0; i < removeObjects.size(); i++) {
			launchObjects.instances.erase(launchObjects.instances.begin() + removeObjects.top());
			removeObjects.pop();
		}

		if (launchObjects.instances.size() > 0) {
			launchObjects.render(shader, dt);
		}

		lampShader.activate();
		lampShader.setMat4("view", view);
		lampShader.setMat4("projection", projection);

		lamps.render(lampShader, dt);

		screen.newFrame();
	}

	lamps.cleanup();

	launchObjects.cleanup();

	glfwTerminate();
	return 0;
}

void launchItem(float dt) {
	RigidBody rb(1.0f, Camera::defaultCamera.cameraPos);
	rb.applyImpulse(Camera::defaultCamera.cameraFront, 1000.0f, dt);
	rb.applyAcceleration(Environment::gravitationalAcceleration);
	launchObjects.instances.push_back(rb);
};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(double dt) {
	if (Keyboard::key(GLFW_KEY_ESCAPE))
		screen.setShouldClose(true);

	if (Keyboard::keyWentDown(GLFW_KEY_F)) {
		flashLightOn = !flashLightOn;
	}

	if (Keyboard::key(GLFW_KEY_W)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::FORWARD, dt);
	}

	if (Keyboard::key(GLFW_KEY_S)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::BACKWARD, dt);
	}

	if (Keyboard::key(GLFW_KEY_A)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::LEFT, dt);
	}

	if (Keyboard::key(GLFW_KEY_D)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::RIGHT, dt);
	}

	if (Keyboard::key(GLFW_KEY_SPACE)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::UP, dt);
	}

	if (Keyboard::key(GLFW_KEY_LEFT_CONTROL)) {
		Camera::defaultCamera.updateCameraPos(CameraDirection::DOWN, dt);
	}

	double dx = Mouse::getDX(), dy = Mouse::getDY();

	if (dx != 0 || dy != 0) {
		Camera::defaultCamera.updateCameraDirection(dx, dy);
	}

	double scrollDy = Mouse::getScrollDY();
	if (scrollDy != 0) {
		Camera::defaultCamera.updateCameraZoom(scrollDy);
	}

	if (Keyboard::keyWentDown(GLFW_KEY_L)) {
		launchItem(dt);
	}
}