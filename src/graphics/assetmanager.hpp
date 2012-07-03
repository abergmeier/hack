#pragma once

#include <map>
#include "asset.hpp"
#include "../logic/entity.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class assetmanager
{
public:
	assetmanager(void);
	~assetmanager(void);
	asset* get(const char *);
private:
	void init();
	std::map<const char*,std::shared_ptr<sf::Texture>> textureMap;
	void addTexture(const char* name, const char* path);
};

