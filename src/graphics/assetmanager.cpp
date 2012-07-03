#include "assetmanager.hpp"
#include "../logic/stone.hpp"
#include <iostream>
using namespace sf;
using namespace hack::logic;

assetmanager::assetmanager(void)
{
	init();
}


assetmanager::~assetmanager(void)
{
	textureMap.clear();
}

void assetmanager::init() {
	addTexture(Stone::NAME.c_str(),"resources/stone.png");
}

void assetmanager::addTexture(const char* name, const char* path) {
	std::shared_ptr<Texture> tex(new Texture());
	tex->loadFromFile(path);
	textureMap[name] = tex;
}

asset* assetmanager::get(const char* name) {
	return new asset(textureMap[name]);
}