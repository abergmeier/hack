#pragma once

#include <map>
#include "asset.hpp"
#include "../logic/entity.hpp"
#include <SFML/Graphics.hpp>

class assetmanager
{
public:
	assetmanager(void);
	~assetmanager(void);
	asset* get(const char *);
private:
	void init();
	std::map<const char*,sf::Texture*> textureMap;
	void addTexture(const char* name, const char* path);
};

