#include "Octree.h"

void Octree::calculateBounds(BoundingRegion* out, Octant octant, BoundingRegion parentRegion) {
	glm::vec3 center = parentRegion.calculateCenter();
	if (octant == Octant::O1) {
		out = new BoundingRegion(center, parentRegion.max);
	}
	else if (octant == Octant::O2) {
		out = new BoundingRegion(glm::vec3(parentRegion.min.x, center.y, center.z), glm::vec3(center.x, parentRegion.max.y, parentRegion.max.z));
	}
	else if (octant == Octant::O3) {
		out = new BoundingRegion(glm::vec3(parentRegion.min.x, parentRegion.min.y, center.z), glm::vec3(center.x, center.y, parentRegion.max.z));
	}
	else if (octant == Octant::O4) {
		out = new BoundingRegion(glm::vec3(center.x, parentRegion.min.y, center.z), glm::vec3(parentRegion.max.x, center.y, parentRegion.max.z));
	}
	else if (octant == Octant::O5) {
		out = new BoundingRegion(glm::vec3(center.x, center.y, parentRegion.min.z), glm::vec3(parentRegion.max.x, parentRegion.max.y, center.z));
	}
	else if (octant == Octant::O6) {
		out = new BoundingRegion(glm::vec3(parentRegion.min.x, center.y, parentRegion.min.z), glm::vec3(center.x, parentRegion.max.y, center.z));
	}
	else if (octant == Octant::O7) {
		out = new BoundingRegion(parentRegion.min, center);
	}
	else if (octant == Octant::O8) {
		out = new BoundingRegion(glm::vec3(center.x, parentRegion.min.y, parentRegion.min.z), glm::vec3(parentRegion.max.x, center.y, center.z));
	}
}

Octree::node::node() : region(BoundTypes::AABB) {

}

Octree::node::node(BoundingRegion bounds) : region(bounds) {

}

Octree::node::node(BoundingRegion bounds, std::vector<BoundingRegion> objectList) : region(bounds) {
	objects.insert(objects.end(), objectList.begin(), objectList.end());
}

void Octree::node::build() {
	if (objects.size() <= 1) {
		return;
	}

	glm::vec3 dimensions = region.calculateDimensions();
	for (int i = 0; i < 3; i++) {
		if (dimensions[i] < MIN_BOUNDS) {
			return;
		}
	}

	// create
	BoundingRegion octants[NO_CHILDREN];
	for (int i = 0; i < NO_CHILDREN; i++) {
		calculateBounds(&octants[i], (Octant)(1 << i), region);
	}

	// puplate
	std::vector<BoundingRegion> octLists[NO_CHILDREN];
	std::stack<int> delList;

	for (int i = 0, length = objects.size(); i < length; i++) {
		BoundingRegion br = objects[i];

		for (int j = 0; j < NO_CHILDREN; j++) {
			if (octants[j].containsRegion(br)) {
				octLists[j].push_back(br);
				delList.push(i);
				break;
			}
		}
	}

	// remove
	while (delList.size() != 0) {
		objects.erase(objects.begin() + delList.top());
		delList.pop();
	}

	// populate
	for (int i = 0; i < NO_CHILDREN; i++) {
		if (octLists[i].size() != 0) {
			children[i] = new node(octants[i], octLists[i]);
			States::activateIndex(&activeOctants, i);
			children[i]->build();
			hasChildren = true;
		}
	}

	treeBuilt = true;
	treeReady = true;
}

// update objects in tree (called during each iteration of main loop)
void Octree::node::update() {
	if (treeBuilt && treeReady) {
		// get moved objects that were in this leaf in previous frame
		std::vector<BoundingRegion> movedObjects(objects.size());
		// TODO

		// remove objects that don't exist anymore
		for (int i = 0, listSize = objects.size(); i < listSize; i++) {
			// remove if on list of dead objects
			// TODO
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
						children[i]->update();
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

			movedObj = movedObjects[0]; // set to first object in list
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

			// remove first object, second object now becomes first
			movedObjects.erase(movedObjects.begin());
			objects.erase(objects.begin() + List::getIndexOf<BoundingRegion>(objects, movedObj));
			current->insert(movedObj);

			// collision detection
			// TODO
		}
	}
	else {
		// process pending results
		if (queue.size() > 0) {
			processPending();
		}
	}
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
		// insert the objects immediately
		while (queue.size() != 0) {
			insert(queue.front());
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
			calculateBounds(&octants[i], (Octant)(1 << i), region);
		}
	}

	// find region that fits item entirely
	for (int i = 0; i < NO_CHILDREN; i++) {
		if (octants[i].containsRegion(obj)) {
			if (children[i] != nullptr) {
				// insert into existing child
				return children[i]->insert(obj);
			}
			else {
				// create node for child
				children[i] = new node(octants[i], { obj });
				States::activateIndex(&activeOctants, i);
				return true;
			}
		}
	}

	// doesn't fit into children
	objects.push_back(obj);
	return true;
}

// destroy object (free memory)
void Octree::node::destroy() {
	// clearing out children
	if (children != nullptr) {
		for (int flags = activeOctants, i = 0;
			flags > 0;
			flags >> 1, i++) {
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