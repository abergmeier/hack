/*
 * objects.hpp
 *
 *  Created on: Jun 22, 2012
 *      Author: andreas
 */

#ifndef OBJECTS_HPP_
#define OBJECTS_HPP_

#include <memory>
#include <map>
#include <vector>
#include "object.hpp"

namespace hack {
namespace logic {

class Objects {
	typedef std::map<id_type, std::weak_ptr<Object>> object_map_type;
	static object_map_type OBJECTS;
	typedef std::function<std::unique_ptr<Object>(std::istream& stream)> deserialize_function_type;
	typedef std::map<std::string, deserialize_function_type> class_map_type;
	static class_map_type CLASS_MAP;
public:
	Objects() = delete;
	template <typename T>
	static void Register() {
		deserialize_function_type func = [](std::istream& stream) -> std::unique_ptr<Object> {
			auto ptr = std::unique_ptr<Object>(new T(stream));
			return ptr;
		};

		CLASS_MAP.insert(std::make_pair(T::NAME, func));
	}

	static std::shared_ptr<Object> Deserialize(std::istream& stream);
};

} }

#endif /* OBJECTS_HPP_ */
