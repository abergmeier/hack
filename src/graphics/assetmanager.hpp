/**
	assetmanager.hpp

	manager where you can load and register textures with there designated name
	it is also a factory to create asset objects out of the collected texture data
*/
#pragma once
#ifndef _ASSETMANAGER_HPP_
#define _ASSETMANAGER_HPP_

#include <map>
#include "asset.hpp"
#include "../logic/entity.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>

namespace hack {
namespace graphics {

/**
	assetmanager class holding a map of texture pointers mapped to a name
*/
class assetmanager
{
public:
	//constructor, destructor
	assetmanager(void);
	~assetmanager(void);

	//generator for assets through a given name
	asset* get(const char *);
private:
	//map of names with textures
	std::map<const char*,std::shared_ptr<sf::Texture>> textureMap;

	//initializing function, texture registration goes here
	void init();

	//number of players on the field, so that each on gets an individual color
	int numPlayer;

	//vector with the names of the colored player sprites
	std::vector<const char*> avatar_names;

	//function to register textures
	void addTexture(const char* name, const char* path);
};

}
}

#endif // !_ASSETMANAGER_HPP_