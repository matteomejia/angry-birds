#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "../objects/Model.h"

class Sphere : public Model {
public:
	Sphere(unsigned int maxNoInstances)
		: Model("sphere", BoundTypes::SPHERE, maxNoInstances, NO_TEX | DYNAMIC) {}

	void init() {
		loadModel("resources/models/sphere/scene.gltf");
	}
};

#endif