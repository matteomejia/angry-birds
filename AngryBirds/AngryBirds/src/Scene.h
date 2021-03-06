#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <map>

#include <glm/glm.hpp>

#include "graphics/rendering/Light.h"
#include "graphics/rendering/Shader.h"
#include "graphics/objects/Model.h"
#include "graphics/models/Box.hpp"

#include "io/camera.h"
#include "io/keyboard.h"
#include "io/mouse.h"

#include "algorithms/states.hpp"
#include "algorithms/Trie.hpp"
#include "algorithms/Octree.h"

namespace Octree {
	class node;
}

class Model;

class Scene {
public:
	trie::Trie<Model*> models;
	trie::Trie<RigidBody*> instances;

	std::vector<RigidBody*> instancesToDelete;

	Octree::node* octree;

	/*
		callbacks
	*/
	// window resize
	static void framebufferSizeCallback(GLFWwindow* widnow, int width, int height);

	/*
		constructor
	*/
	Scene();
	Scene(int glfwVersionMajor, int glfwVersionMinor,
		const char* title, unsigned int scrWidth, unsigned int scrHeight);

	/*
		initialization
	*/
	bool init();

	void prepare(Box &box);

	/*
		main loop methods
	*/
	// process input
	void processInput(float dt);

	// update screen before each frame
	void update();

	// update screen after frame
	void newFrame(Box &box);

	// set uniform shader varaibles (lighting, etc)
	void renderShader(Shader shader, bool applyLighting = true);

	void renderInstances(std::string modelId, Shader shader, float dt);

	/*
		cleanup method
	*/
	void cleanup();

	/*
		accessors
	*/
	bool shouldClose();

	Camera* getActiveCamera();

	/*
		modifiers
	*/
	void setShouldClose(bool shouldClose);

	void setWindowColor(float r, float g, float b, float a);

	/*
		Model/instance methods
	*/
	void registerModel(Model* model);

	RigidBody* generateInstance(std::string modelId, glm::vec3 size, float mass, glm::vec3 pos);

	void initInstances();

	void loadModels();

	void removeInstance(std::string instanceId);

	void markForDeletion(std::string instanceId);

	void clearDeadInstances();

	std::string currentId;
	std::string generateId();

	/*
		lights
	*/
	// list of point lights
	std::vector<PointLight*> pointLights;
	unsigned int activePointLights;
	// list of spot lights
	std::vector<SpotLight*> spotLights;
	unsigned int activeSpotLights;
	// direction light
	DirLight* dirLight;
	bool dirLightActive;

	/*
		camera
	*/
	std::vector<Camera*> cameras;
	unsigned int activeCamera;
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec3 cameraPos;

protected:
	// window object
	GLFWwindow* window;

	// window vals
	const char* title;
	static unsigned int scrWidth;
	static unsigned int scrHeight;

	float bg[4]; // background color

	// GLFW info
	int glfwVersionMajor;
	int glfwVersionMinor;
};

#endif