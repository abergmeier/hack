
#include <istream>
#include "entity.hpp"

using namespace hack::logic;

entity::entity() {
}

id_type::id_type(std::string global, size_t local) :
	global_id(global),
	local_id(local)
{
}

id_type::id_type(std::istream& stream) {
	stream >> *this;
}

bool id_type::operator <(const id_type& other) const {
	if( global_id == other.global_id )
		return local_id < other.local_id;

	return global_id < other.global_id;
}

id_type::id_type(const id_type& other) :
	global_id(other.global_id),
	local_id(other.local_id)
{
}

id_type::id_type(id_type&& other) :
	global_id(std::move(other.global_id)),
	local_id(std::move(other.local_id))
{
}

id_type& id_type::operator=(const id_type& other) {
	global_id = other.global_id;
	local_id = other.local_id;
	return *this;
}


id_type& id_type::operator=(id_type&& other) {
	global_id = std::move(other.global_id);
	local_id = std::move(other.local_id);
	return *this;
}

std::ostream& hack::logic::operator <<(std::ostream& stream, const id_type& id) {
	return stream << '{'
	               << id.global_id << ','
	               << id.local_id
	               << '}';
}

std::istream& hack::logic::operator >>(std::istream& stream, id_type& id) {
	char term;
	stream .get(term); //{
	stream >> id.global_id;
	stream .get(term); //,
	stream >> id.local_id;
	stream .get(term); //}
	return stream;
}


