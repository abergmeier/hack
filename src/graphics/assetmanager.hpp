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

class assetmanager
{
public:
	assetmanager(void);
	~assetmanager(void);
	asset* get(const char *);
private:
	void init();
	std::map<const char*,std::shared_ptr<sf::Texture>> textureMap;
	int numPlayer;
	std::vector<const char*> avatar_names;
	void addTexture(const char* name, const char* path);
};

}
}

#endif // !_ASSETMANAGER_HPP_