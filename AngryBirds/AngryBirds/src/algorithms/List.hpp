#ifndef LIST_HPP
#define LIST_HPP

#include <vector>

namespace List {
	template<typename T>
	int getIndexOf(std::vector<T> v, T x) {
		return std::find(v.begin(), v.end(), x) - v.begin();
	}

	template<typename T>
	bool contains(std::vector<T> v, T x) {
		return std::find(v.begin(), v.end(), x) != v.end();
	}
}

#endif