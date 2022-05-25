#ifndef GUN_HPP
#define GUN_HPP

#include "../objects/Model.h"
#include "../../io/Camera.h"
#include "../../io/Keyboard.h"

#include "../../Scene.h"

class Gun : public Model {
public:
	Gun()
		: Model(BoundTypes::AABB, glm::vec3(0.0f), glm::vec3(1 / 300.0f), true) {}

	void render(Shader shader, float dt, Box* box, Scene* scene, bool setModel = false) {
		glm::mat4 model = glm::mat4(1.0f);

		// position
		rb.pos = scene->getActiveCamera()->cameraPos + glm::vec3(scene->getActiveCamera()->cameraFront * 0.5f) - glm::vec3(scene->getActiveCamera()->cameraUp * 0.205f);
		model = glm::translate(model, rb.pos);

		float theta;

		// rotate around camera
		theta = acos(glm::dot(scene->getActiveCamera()->worldUp, scene->getActiveCamera()->cameraFront) /
			glm::length(scene->getActiveCamera()->cameraUp) / glm::length(scene->getActiveCamera()->cameraFront));
		model = glm::rotate(model, atanf(1) * 2 - theta, scene->getActiveCamera()->cameraRight);

		// rotation
		glm::vec2 front2D = glm::vec2(scene->getActiveCamera()->cameraFront.x, scene->getActiveCamera()->cameraFront.z);
		theta = acos(glm::dot(glm::vec2(1.0f, 0.0f), front2D) / glm::length(front2D));
		model = glm::rotate(model, scene->getActiveCamera()->cameraFront.z < 0 ? theta : -theta, scene->getActiveCamera()->worldUp);


		model = glm::scale(model, size);

		shader.setMat4("model", model);

		Model::render(shader, dt, box, false);
	}
};

#endif