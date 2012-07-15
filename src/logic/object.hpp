/*
 * object.hpp
 *
 *  Created on: Jun 16, 2012
 *      Author: andreas
 */

#ifndef _OBJECT_HPP_
#define _OBJECT_HPP_

#include <istream>
#include <ostream>
#include <array>
#include <string>
#include <cmath>
#include "entity.hpp"
#include "../state/serializable.hpp"

namespace hack {
namespace logic {

class Object : public entity, public hack::state::Serializable {
public:
	virtual ~Object();
	virtual Object& operator = (std::istream& stream);

	const id_type& getid() const;
	int getX() const;
	int getY() const;
	float getAngle() const;
	size_t getHeight() const;
	size_t getWidth() const;

	void setX(int value);
	void setY(int value);
	void setAngle(float value);
	void setWidth(size_t value);
	void setHeight(size_t value);


private:
	const id_type id;
	int x;
	int y;
	float angle;
	size_t width;
	size_t height;

	// Sets all non const fields
	void SetNonConst(std::istream& stream);

protected:
	Object(std::string siteID);
	// Use this constructor to deserialize object
	Object(std::istream& stream);
	virtual std::ostream& SerializeContent(std::ostream& stream) const override;
};

template <typename T>
struct vector2 : protected std::array<T, 2> {
	typedef std::array<T, 2> __BASE;
	typedef typename __BASE::reference reference;
	typedef typename __BASE::const_reference const_reference;
	typedef typename __BASE::size_type size_type;

	vector2() :
		__BASE()
	{
	}

	vector2(T first, T second) :
		__BASE()
	{
		(*this)[0] = first;
		(*this)[1] = second;
	}

	template <typename OT>
	T dot( const OT& other ) const {
		T result = 0;
		for( size_t i = 0; i != std::tuple_size<__BASE>::value; ++i ) {
			result += (*this)[i] * other[i];
		}
		return result;
	}

	reference operator[]( size_type pos ) {
		return __BASE::operator[]( pos );
	}
	const_reference operator[]( size_type pos ) const {
		return __BASE::operator[]( pos );
	}

	double length() const {
		double result = 0;
		for( size_t i = 0; i != std::tuple_size<__BASE>::value; ++i ) {
			result += std::pow( (*this)[i], 2 );
		}
		return std::sqrt( result );
	}

	template <typename OT>
	vector2<T> sub(const OT& other) {
		return vector2<T>((*this)[0]-other[0],(*this)[1]-other[1]);
	}

	template <typename OT>
	vector2<T> add(const OT& other) {
		return vector2<T>((*this)[0]+other[0],(*this)[1]+other[1]);
	}
};

} }


#endif // _OBJECT_HPP_
