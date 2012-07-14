
#include <istream>
#include "entity.hpp"
#include "../state/serializable.hpp"

using namespace hack::logic;

id_type::id_type(std::string global, size_t local) :
	global_id(global),
	local_id(local)
{
}

id_type::id_type(std::istream& stream) {
	stream >> *this;
}

bool id_type::operator <(const id_type& other) const {
	if( global_id != other.global_id )
		return global_id < other.global_id;

	return local_id < other.local_id;
}

bool id_type::operator ==(const id_type& other) const {
	return global_id == other.global_id
	    && local_id == other.local_id;
}

bool id_type::operator !=(const id_type& other) const {
	return !( *this == other);
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
	stream << '{';
	hack::state::Serializable::String::Serialize( stream, id.global_id );
	stream << ','
	       << id.local_id
	       << '}';
	return stream;
}

std::istream& hack::logic::operator >>(std::istream& stream, id_type& id) {
	stream.get(); //{
	id.global_id = hack::state::Serializable::String::Deserialize( stream );
	stream.get(); //,
	stream >> id.local_id;
	stream.get(); //}
	return stream;
}

bool hack::logic::entity::operator==(const entity& other) const {
	return getid() == other.getid();
}

bool hack::logic::entity::operator!=(const entity& other) const {
	return getid() == other.getid();
}


