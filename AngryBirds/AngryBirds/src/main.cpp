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

#include "algorithms/States.hpp"

#include "graphics/objects/Model.h"
#include "graphics/models/Cube.hpp"
#include "graphics/models/Lamp.hpp"
#include "graphics/models/Gun.hpp"
#include "graphics/models/Sphere.hpp"
#include "graphics/models/Box.hpp"
#include "graphics/rendering/Shader.h"
#include "graphics/rendering/Texture.h"
#include "graphics/rendering/Light.h"

#include "io/Keyboard.h"
#include "io/Mouse.h"
#include "io/Gamepad.h"
#include "io/Camera.h"

#include "physics/Environment.h"

#include "Scene.h"

Scene scene;

void processInput(double dt);

Camera cam;

//Joystick mainJ(0);

double dt = 0.0f; // tme btwn frames
double lastFrame = 0.0f; // time of last frame

Sphere sphere(10);

int main() {
	scene = Scene(3, 3, "Angry Birds", 800, 600);
	if (!scene.init()) {
		std::cout << "Could not open window" << std::endl;
		glfwTerminate();
		return -1;
	}

	scene.cameras.push_back(&cam);
	scene.activeCamera = 0;

	// SHADERS===============================
	Shader lampShader("resources/shaders/instanced/instanced.vert", "resources/shaders/lamp.frag");
	Shader shader("resources/shaders/instanced/instanced.vert", "resources/shaders/object.frag");
	Shader boxShader("resources/shaders/instanced/box.vert", "resources/shaders/instanced/box.frag");

	// MODELS==============================
	Lamp lamp(4);
	scene.registerModel(&lamp);

	scene.registerModel(&sphere);

	scene.loadModels();

	// LIGHTS
	DirLight dirLight = { glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec4(0.1f, 0.1f, 0.1f, 1.0f), glm::vec4(0.4f, 0.4f, 0.4f, 1.0f), glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) };
	scene.dirLight = &dirLight;

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

	PointLight pointLights[4];

	for (unsigned int i = 0; i < 4; i++) {
		pointLights[i] = {
			pointLightPositions[i],
			k0, k1, k2,
			ambient, diffuse, specular
		};
		scene.generateInstance(lamp.id, glm::vec3(0.25f), 0.25f, pointLightPositions[i]);
		scene.pointLights.push_back(&pointLights[i]);
		States::activate(&scene.activePointLights, i);
	}

	SpotLight spotLight = {
		cam.cameraPos, cam.cameraFront,
		glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(20.0f)),
		1.0f, 0.07f, 0.032f,
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f), glm::vec4(1.0f)
	};
	scene.spotLights.push_back(&spotLight);
	scene.activeSpotLights = 1;	// 0b00000001

	// instantiate instances
	scene.initInstances();

	// joystick recognition
	/*mainJ.update();
	if (mainJ.isPresent()) {
		std::cout << mainJ.getName() << " is present." << std::endl;
	}*/

	while (!scene.shouldClose()) {
		// calculate dt
		double currentTime = glfwGetTime();
		dt = currentTime - lastFrame;
		lastFrame = currentTime;

		// update screen values
		scene.update();

		// process input
		processInput(dt);

		// remove launch objects if too far
		std::stack<unsigned int> removeObjects;
		for (int i = 0; i < sphere.currentNoInstances; i++) {
			if (glm::length(cam.cameraPos - sphere.instances[i].pos) > 250.0f) {
				removeObjects.push(i);
			}
		}
		while (removeObjects.size() != 0) {
			sphere.removeInstance(removeObjects.top());
			removeObjects.pop();
		}

		// render launch objects
		if (sphere.currentNoInstances > 0) {
			scene.renderShader(shader);
			scene.renderInstances(sphere.id, shader, dt);
		}

		// render lamps
		scene.renderShader(lampShader);
		scene.renderInstances(lamp.id, lampShader, dt);

		// send new frame to window
		scene.newFrame();
	}

	scene.cleanup();
	return 0;
}

void launchItem(float dt) {
	std::string id = scene.generateInstance(sphere.id, glm::vec3(1.0f), 1.0f, cam.cameraPos);
	if (id != "") {
		// instance generated
		sphere.instances[scene.instances[id].second].transferEnergy(100.0f, cam.cameraFront);
		sphere.instances[scene.instances[id].second].applyAcceleration(Environment::gravitationalAcceleration);
	}
}

void processInput(double dt) {
	scene.processInput(dt);

	// update flash light
	if (States::isIndexActive(&scene.activeSpotLights, 0)) {
		scene.spotLights[0]->position = scene.getActiveCamera()->cameraPos;
		scene.spotLights[0]->direction = scene.getActiveCamera()->cameraFront;
	}

	if (Keyboard::key(GLFW_KEY_ESCAPE)) {
		scene.setShouldClose(true);
	}

	if (Keyboard::keyWentDown(GLFW_KEY_F)) {
		States::toggleIndex(&scene.activeSpotLights, 0); // toggle spot light
	}

	if (Mouse::buttonWentDown(GLFW_MOUSE_BUTTON_1)) {
		launchItem(dt);
	}

	for (int i = 0; i < 4; i++) {
		if (Keyboard::keyWentDown(GLFW_KEY_1 + i)) {
			States::toggleIndex(&scene.activePointLights, i);
		}
	}
}