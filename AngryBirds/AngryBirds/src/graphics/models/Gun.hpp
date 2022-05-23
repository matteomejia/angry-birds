#ifndef GUN_HPP
#define GUN_HPP

#include "../objects/Model.h"
#include "../../io/Camera.h"

class Gun : public Model {
public:
	Gun() : Model(glm::vec3(0.0f), glm::vec3(1 / 300.0f), true) {}

	void render(Shader shader, float dt, bool setModel = false) {
		glm::mat4 model = glm::mat4(1.0f);

		// position
		rb.pos = Camera::defaultCamera.cameraPos + glm::vec3(Camera::defaultCamera.cameraFront * 0.5f) - glm::vec3(Camera::defaultCamera.cameraUp * 0.205f);
		model = glm::translate(model, rb.pos);

		float theta;

		// rotate around camera
		theta = acos(glm::dot(Camera::defaultCamera.worldUp, Camera::defaultCamera.cameraFront) /
			glm::length(Camera::defaultCamera.cameraUp) / glm::length(Camera::defaultCamera.cameraFront));
		model = glm::rotate(model, atanf(1) * 2 - theta, Camera::defaultCamera.cameraRight);

		// rotation
		glm::vec2 front2D = glm::vec2(Camera::defaultCamera.cameraFront.x, Camera::defaultCamera.cameraFront.z);
		theta = acos(glm::dot(glm::vec2(1.0f, 0.0f), front2D) / glm::length(front2D));
		model = glm::rotate(model, Camera::defaultCamera.cameraFront.z < 0 ? theta : -theta, Camera::defaultCamera.worldUp);


		model = glm::scale(model, size);

		shader.setMat4("model", model);

		Model::render(shader, dt, false);
	}
};

#endif