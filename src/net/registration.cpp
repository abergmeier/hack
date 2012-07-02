
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <iostream>
#include <sstream>
#include <json/json.h>
#include "../debug.hpp"
#include "registration.hpp"

using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequest;
using Poco::Net::HTTPMessage;
using Poco::Net::HTTPResponse;

namespace {

	struct Debug : public hack::Debug {
		const std::string& GetCategory() const override {
			static const std::string CATEGORY = "Reg";
			return CATEGORY;
		}
	};

	static const Debug DEBUG;

	static const std::string HOST("localhost");
	static const size_t      PORT(4242);

	struct ResponseException : public std::runtime_error {
		ResponseException( const Poco::Net::HTTPResponse& res, const std::string& message ) :
			std::runtime_error(message)
		{
			DEBUG.ERR_ENTRY( std::stringstream() << "Status of HttpResponse: " << res.getStatus() << " " << res.getReason() );
		}
	};

	std::string ProcessResponse( Poco::Net::HTTPClientSession& session ) {
		HTTPResponse res;

		// print response
		auto& is = session.receiveResponse( res );
		//std::string response;
		//is >> response;

		auto value = res.getStatus();
		switch( value ) {
		case HTTPResponse::HTTPStatus::HTTP_OK:
		{
			std::stringstream stream;
			stream << is.rdbuf();
			return stream.str();
		}
		case HTTPResponse::HTTPStatus::HTTP_BAD_GATEWAY:
			throw ResponseException( res, "Bad Gateway" );
		case HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST:
			throw ResponseException( res, "Bad Request" );
		case HTTPResponse::HTTPStatus::HTTP_CONFLICT:
			throw ResponseException( res, "Conflict" );
		case HTTPResponse::HTTPStatus::HTTP_FORBIDDEN:
			throw ResponseException( res, "Forbidden" );
		case HTTPResponse::HTTPStatus::HTTP_GATEWAY_TIMEOUT:
			throw ResponseException( res, "Gateway Timeout" );
		case HTTPResponse::HTTPStatus::HTTP_GONE:
			throw ResponseException( res, "Gone" );
		case HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR:
			throw ResponseException( res, "Internal Server Error" );
		case HTTPResponse::HTTPStatus::HTTP_LENGTH_REQUIRED:
			throw ResponseException( res, "Length Required" );
		case HTTPResponse::HTTPStatus::HTTP_METHOD_NOT_ALLOWED:
			throw ResponseException( res, "Method not allowed" );
		case HTTPResponse::HTTPStatus::HTTP_NOT_FOUND:
			throw ResponseException( res, "Not found" );
		case HTTPResponse::HTTPStatus::HTTP_NOT_IMPLEMENTED:
			throw ResponseException( res, "Not implemented" );
		case HTTPResponse::HTTPStatus::HTTP_REQUEST_TIMEOUT:
			throw ResponseException( res, "Request timeout" );
		case HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED:
			throw ResponseException( res, "Unauthorized" );
		default:
			throw ResponseException( res, "REQUEST failed" );
		}
	}


	std::string CreateRequest( const std::string& method, const std::string& uri ) {
		HTTPClientSession session( HOST, PORT );

		HTTPRequest req( method, uri, Poco::Net::HTTPMessage::HTTP_1_1 );
		session.sendRequest( req );

		return ProcessResponse( session );
	}

	std::string CreatePost( const std::string& uri, const std::string& body ) {
		HTTPClientSession session( HOST, PORT );

		HTTPRequest req( HTTPRequest::HTTP_POST, uri, HTTPMessage::HTTP_1_1 );
		//req.setContentType( "application/x-www-form-urlencoded\r\n" );
		//req.setKeepAlive  ( true );
		session.sendRequest( req ) << body;

		return ProcessResponse( session );
	}

}

using namespace hack::net;

Registration::Registration(std::string uuid, port_type port) :
	_uuid(uuid),
	_uri([&uuid]() -> std::string {
		std::stringstream stream;
		//stream << URI;
		stream << '/' << uuid;
		return stream.str();
	}())
{
	std::stringstream body;
	body << "host=localhost&port=" << port;
	std::string response = CreatePost( _uri, body.str() );
}

Registration::~Registration() {
	try {
		CreateRequest( HTTPRequest::HTTP_DELETE, _uri );
	} catch( const std::exception& e ) {

	}
}

std::vector<Registration::Element> Registration::GetAll() {
	std::string response = CreateRequest( HTTPRequest::HTTP_GET, "/" );

	std::vector<Element> others;

	Json::Value root; // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse( response, root );
	if( !parsingSuccessful ) {
	    // report to the user the failure and their locations in the document.
	    DEBUG.ERR_ENTRY( std::stringstream() << "Failed to parse configuration" << std::endl
	                     << reader.getFormattedErrorMessages() );
	    return others;
	}

	for( size_t index = 0; index < root.size(); ++index )  // Iterates over the sequence elements.
	{
		auto& object = root[static_cast<int>(index)];

		Element element;
		element.uuid = object["uuid"].asString();
		element.host = object["host"].asString();
		element.port = object.get("port", 0).asInt();
		others.push_back( std::move(element) );
	}

	return others;
}