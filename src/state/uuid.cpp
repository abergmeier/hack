/*
 * uuid.cpp
 *
 *  Created on: Jun 26, 2012
 *      Author: andreas
 */

#ifdef noexcept
#undef noexcept
#endif //noexcept

#ifdef _MSC_VER
	#define _WINSOCKAPI_
	#include <windows.h>
	#include <Rpc.h>
#else
	#include <uuid/uuid.h>
#endif // DEBUG
#include "uuid.hpp"

using namespace hack;

std::string hack::UUID::Generate() {
#ifdef _MSC_VER
		::UUID id;
		RPC_CSTR cstr;
		UuidCreate(&id);
		UuidToStringA(&id,&cstr);
		std::string uuid(reinterpret_cast<char*>(cstr));
		RpcStringFreeA(&cstr);
#else
		// We have to use array here because
		// std::string does not provide access to
		// internal char array
		std::array<char, 16> uuidBuffer;

		// Generate function wants buffer unsigned but string does only accept signed.
		// Since sizes are the same - cast to unsigned for function.
		uuid_generate_random(reinterpret_cast<unsigned char*>(uuidBuffer.data()));
		std::string uuid(uuidBuffer.data(), uuidBuffer.size());
#endif // _MSC_VER
		return uuid;
}


