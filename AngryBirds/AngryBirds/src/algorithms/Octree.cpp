#include "Octree.h"

#include "../graphics/models/Box.hpp"

void Octree::calculateBounds(BoundingRegion& out, Octant octant, BoundingRegion parentRegion) {
	glm::vec3 center = parentRegion.calculateCenter();
	if (octant == Octant::O1) {
		out = BoundingRegion(center, parentRegion.max);
	}
	else if (octant == Octant::O2) {
		out = BoundingRegion(glm::vec3(parentRegion.min.x, center.y, center.z), glm::vec3(center.x, parentRegion.max.y, parentRegion.max.z));
	}
	else if (octant == Octant::O3) {
		out = BoundingRegion(glm::vec3(parentRegion.min.x, parentRegion.min.y, center.z), glm::vec3(center.x, center.y, parentRegion.max.z));
	}
	else if (octant == Octant::O4) {
		out = BoundingRegion(glm::vec3(center.x, parentRegion.min.y, center.z), glm::vec3(parentRegion.max.x, center.y, parentRegion.max.z));
	}
	else if (octant == Octant::O5) {
		out = BoundingRegion(glm::vec3(center.x, center.y, parentRegion.min.z), glm::vec3(parentRegion.max.x, parentRegion.max.y, center.z));
	}
	else if (octant == Octant::O6) {
		out = BoundingRegion(glm::vec3(parentRegion.min.x, center.y, parentRegion.min.z), glm::vec3(center.x, parentRegion.max.y, center.z));
	}
	else if (octant == Octant::O7) {
		out = BoundingRegion(parentRegion.min, center);
	}
	else if (octant == Octant::O8) {
		out = BoundingRegion(glm::vec3(center.x, parentRegion.min.y, parentRegion.min.z), glm::vec3(parentRegion.max.x, center.y, center.z));
	}
}

Octree::node::node() : region(BoundTypes::AABB) {

}

Octree::node::node(BoundingRegion bounds) : region(bounds) {

}

Octree::node::node(BoundingRegion bounds, std::vector<BoundingRegion> objectList) : region(bounds) {
	objects.insert(objects.end(), objectList.begin(), objectList.end());
}

void Octree::node::addToPending(RigidBody* instance, trie::Trie<Model*> models) {
	for (BoundingRegion br : models[instance->modelId]->boundingRegions) {
		br.instance = instance;
		br.transform();
		queue.push(br);
	}
}

void Octree::node::build() {
	// variables
	glm::vec3 dimensions = region.calculateDimensions();
	BoundingRegion octants[NO_CHILDREN];
	std::vector<BoundingRegion> octLists[NO_CHILDREN];

	if (objects.size() <= 1) {
		goto setVars;
	}

	for (int i = 0; i < 3; i++) {
		if (dimensions[i] < MIN_BOUNDS) {
			goto setVars;
		}
	}

	// create	
	for (int i = 0; i < NO_CHILDREN; i++) {
		calculateBounds(octants[i], (Octant)(1 << i), region);
	}

	// populate
	for (int i = 0, length = objects.size(); i < length; i++) {
		BoundingRegion br = objects[i];

		for (int j = 0; j < NO_CHILDREN; j++) {
			if (octants[j].containsRegion(br)) {
				octLists[j].push_back(br);
				objects.erase(objects.begin() + i);
				i--;
				length--;
				break;
			}
		}
	}

	// populate
	for (int i = 0; i < NO_CHILDREN; i++) {
		if (octLists[i].size() != 0) {
			children[i] = new node(octants[i], octLists[i]);
			States::activateIndex(&activeOctants, i);
			children[i]->parent = this;
			children[i]->build();
		}
	}

setVars:
	treeBuilt = true;
	treeReady = true;

	for (int i = 0; i < objects.size(); i++) {
		objects[i].cell = this;
	}
}

// update objects in tree (called during each iteration of main loop)
void Octree::node::update(Box& box) {
	if (treeBuilt && treeReady) {
		box.positions.push_back(region.calculateCenter());
		box.sizes.push_back(region.calculateDimensions());

		if (objects.size() == 0) {
			if (!activeOctants) {
				if (currentLifeSpan == -1) {
					currentLifeSpan = maxLifeSpan;
				}
				else if (currentLifeSpan > 0) {
					currentLifeSpan--;
				}
			}
		}
		else {
			if (currentLifeSpan != -1) {
				if (maxLifeSpan <= 64) {
					maxLifeSpan <<= 2;
				}
			}
		}

		// remove objects that don't exist anymore
		for (int i = 0, listSize = objects.size(); i < listSize; i++) {
			// remove if on list of dead objects
			if (States::isActive(&objects[i].instance->state, INSTANCE_DEAD)) {
				objects.erase(objects.begin() + i);
				i--;
				listSize--;
			}
		}

		// get moved objects that were in this leaf in previous frame
		std::stack<int> movedObjects;
		for (int i = 0, listSize = objects.size(); i < listSize; i++) {
			if (States::isActive(&objects[i].instance->state, INSTANCE_MOVED)) {
				objects[i].transform();
				movedObjects.push(i);
			}
			box.positions.push_back(objects[i].calculateCenter());
			box.sizes.push_back(objects[i].calculateDimensions());
		}

		// dead ranges
		unsigned char flags = activeOctants;
		for (int i = 0; flags > 0; flags >>= 1, i++) {
			if (States::isIndexActive(&flags, 0) && children[i]->currentLifeSpan == 0) {
				if (children[i]->objects.size() > 0) {
					children[i]->currentLifeSpan = -1;
				}
				else {
					free(children[i]);
					children[i] = nullptr;
					States::deactivateIndex(&activeOctants, i);
				}
			}
		}

		// update child nodes
		if (children != nullptr) {
			// go through each octant using flags
			for (unsigned char flags = activeOctants, i = 0;
				flags > 0;
				flags >>= 1, i++) {
				if (States::isIndexActive(&flags, 0)) {
					// active octant
					if (children[i] != nullptr) {
						// child not null
						children[i]->update(box);
					}
				}
			}
		}

		// move moved objects into new nodes
		BoundingRegion movedObj; // placeholder
		while (movedObjects.size() != 0) {
			/*
				for each moved object
				- traverse up tree (start with current node) until find a node that completely encloses the object
				- call insert (push object as far down as possible)
			*/

			movedObj = objects[movedObjects.top()]; // set to first object in list
			node* current = this; // placeholder

			while (!current->region.containsRegion(movedObj)) {
				if (current->parent != nullptr) {
					// set current to current's parent (recursion)
					current = current->parent;
				}
				else {
					break; // if root node, the leave
				}
			}

			// finished
			objects.erase(objects.begin() + movedObjects.top());
			movedObjects.pop();
			current->queue.push(movedObj);

			// collision detection
			current = movedObj.cell;
			current->checkCollisionsSelf(movedObj);

			// children
			current->checkCollisionsChildren(movedObj);

			// parent
			while (current->parent) {
				current = current->parent;
				current->checkCollisionsSelf(movedObj);
			}
		}
	}
	
	processPending();
}

void Octree::node::processPending() {
	if (!treeBuilt) {
		// add objects to be sorted into branches when built
		while (queue.size() != 0) {
			objects.push_back(queue.front());
			queue.pop();
		}
		build();
	}
	else {
		for (int i = 0, len = queue.size(); i < len; i++) {
			BoundingRegion br = queue.front();
			if (region.containsRegion(br)) {
				insert(br);
			}
			else {
				br.transform();
				queue.push(br);
			}
			queue.pop();
		}
	}
}

// dynamically insert object into node
bool Octree::node::insert(BoundingRegion obj) {
	/*
		termination conditions
		- no objects (an empty leaf node)
		- dimensions are less than MIN_BOUNDS
	*/

	glm::vec3 dimensions = region.calculateDimensions();
	if (objects.size() == 0 ||
		dimensions.x < MIN_BOUNDS ||
		dimensions.y < MIN_BOUNDS ||
		dimensions.z < MIN_BOUNDS
		) {
		obj.cell = this;
		objects.push_back(obj);
		return true;
	}

	// safeguard if object doesn't fit
	if (!region.containsRegion(obj)) {
		return parent == nullptr ? false : parent->insert(obj);
	}

	// create regions if not defined
	BoundingRegion octants[NO_CHILDREN];
	for (int i = 0; i < NO_CHILDREN; i++) {
		if (children[i] != nullptr) {
			// child exists, so take its region
			octants[i] = children[i]->region;
		}
		else {
			// get region for this octant
			calculateBounds(octants[i], (Octant)(1 << i), region);
		}
	}

	objects.push_back(obj);

	std::vector<BoundingRegion> octLists[NO_CHILDREN];
	for (int i = 0, length = objects.size(); i < length; i++) {
		objects[i].cell = this;
		for (int j = 0; j < NO_CHILDREN; j++) {
			if (octants[j].containsRegion(objects[i])) {
				octLists[j].push_back(objects[i]);
				objects.erase(objects.begin() + i);
				i--;
				length--;
				break;
			}
		}
	}

	for (int i = 0; i < NO_CHILDREN; i++) {
		if (octLists[i].size() != 0) {
			if (children[i]) {
				for (BoundingRegion br : octLists[i]) {
					children[i]->insert(br);
				}
			}
			else {
				children[i] = new node(octants[i], octLists[i]);
				children[i]->parent = this;
				States::activateIndex(&activeOctants, i);
				children[i]->build();
			}
		}
	}

	return true;
}

void Octree::node::checkCollisionsSelf(BoundingRegion obj) {
	for (BoundingRegion br : objects) {
		if (br.intersectsWith(obj)) {
			if (br.instance->instanceId != obj.instance->instanceId) {
				std::cout << "Instance " << br.instance->instanceId << "(" << br.instance->modelId << ") collides with " << obj.instance->instanceId << "(" << obj.instance->modelId << ")" << std::endl;
			}
		}
	}
}

void Octree::node::checkCollisionsChildren(BoundingRegion obj) {
	if (children) {
		for (int flags = activeOctants, i = 0;
			flags > 0;
			flags >>= 1, i++) {
			if (States::isIndexActive(&flags, 0) && children[i]) {
				children[i]->checkCollisionsSelf(obj);
				children[i]->checkCollisionsChildren(obj);
			}
		}
	}
}

// destroy object (free memory)
void Octree::node::destroy() {
	// clearing out children
	if (children != nullptr) {
		for (int flags = activeOctants, i = 0;
			flags > 0;
			flags >>= 1, i++) {
			if (States::isActive(&flags, 0)) {
				// active
				if (children[i] != nullptr) {
					children[i]->destroy();
					children[i] = nullptr;
				}
			}
		}
	}

	// clear this node
	objects.clear();
	while (queue.size() != 0) {
		queue.pop();
	}
}