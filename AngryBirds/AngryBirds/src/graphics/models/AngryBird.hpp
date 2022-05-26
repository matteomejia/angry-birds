#ifndef ANGRYBIRD_HPP
#define ANGRYBIRD_HPP

#include "../objects/Model.h"

class AngryBird : public Model {
public:
	AngryBird(unsigned int maxNoInstances)
		: Model("angryBird", BoundTypes::AABB, maxNoInstances, DYNAMIC) {}

	void init() {
		loadModel("resources/models/red/scene.gltf");
	}
};

#endif