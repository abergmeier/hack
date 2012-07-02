#include "assetmanager.hpp"

using namespace sf;

assetmanager::assetmanager(void)
{
	init();
}


assetmanager::~assetmanager(void)
{
	textureMap.clear();
}

void assetmanager::init() {
	addTexture("Stone","resources/stone.png");
}

void assetmanager::addTexture(const char* name, const char* path) {
	Texture* tex = new Texture();
	tex->loadFromFile(path);
	textureMap[name] = tex;
}

asset* assetmanager::get(const char* name) {
	return new asset(textureMap[name]);
}