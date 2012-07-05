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
#include <list>
#include "object.hpp"

namespace hack {
namespace logic {

class Objects {
public:
	typedef std::shared_ptr<entity>                value_type;
	typedef std::weak_ptr<entity>                  internal_value_type;
	typedef std::map<id_type, internal_value_type> object_map_type;

	class iterator {
		object_map_type::iterator _it;
	public:
		iterator( object_map_type::iterator it );
	};

	class const_iterator {
		object_map_type::const_iterator _it;
	public:
		const_iterator( object_map_type::const_iterator it );
	};
private:
	typedef value_type                                                   key_type;
	typedef object_map_type::size_type                                   size_type;
	typedef std::function<std::unique_ptr<Object>(std::istream& stream)> deserialize_function_type;
	typedef std::map<std::string, deserialize_function_type>             class_map_type;

	object_map_type _objectMap;

	// Makes sure handler lives as long as we use _handlerCallback
	std::function<void(const internal_value_type&)> _insertHandlerCallback;
	std::function<void(const internal_value_type&)> _eraseHandlerCallback;

	static class_map_type CLASS_MAP;

	std::pair<iterator, bool> insert( const value_type& value );
	size_type                 erase( const key_type& key );

public:
	template <typename T>
	static void Register() {
		deserialize_function_type func = [](std::istream& stream) -> std::unique_ptr<Object> {
			auto ptr = std::unique_ptr<Object>(new T(stream));
			return ptr;
		};

		CLASS_MAP.insert(std::make_pair(T::NAME, func));
	}
/*
	iterator       begin();
	const_iterator begin() const;
	const_iterator cbegin() const;

	iterator       end();
	const_iterator end() const;
	const_iterator cend() const;
*/
	value_type Deserialize(std::istream& stream);

	// Beware: This will crash if you give a null
	// shared ptr
	Objects() :
		_objectMap(),
		_insertHandlerCallback(),
		_eraseHandlerCallback()
	{
	}

	template <typename T>
	void SetCallback( std::weak_ptr<T> weakCollectible ) {
		_insertHandlerCallback = [weakCollectible]( const internal_value_type& value) mutable {
			auto collectible = weakCollectible.lock();

			if( !collectible )
				return;

			collectible->insert( value );
		};
		_eraseHandlerCallback = [weakCollectible]( const internal_value_type& value) mutable {
			auto collectible = weakCollectible.lock();

			if( !collectible )
				return;

			collectible->erase( value );
		};
	}

	void Register( value_type object);
	void Unregister( value_type object);
};

} }

#endif /* OBJECTS_HPP_ */
