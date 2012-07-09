#include "assetmanager.hpp"
#include "../logic/stone.hpp"
#include "../logic/avatar.hpp"
#include <iostream>
#include <string>

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
	//initializing all 8 playercolors
	const char *player_colors[] = {"normal","red","blue","green","yellow","mono","brown","purple"};
	std::string p = Avatar::NAME; 
	std::string resource_start = "resources/avatar-";
	std::string resource_end = ".png";
	for(auto k : player_colors) {
		const char* temp = (p+k).c_str();
		avatar_names.push_back(temp);
		addTexture(temp,(resource_start+k+resource_end).c_str());
	}
	
	//stone texture
	addTexture(Stone::NAME.c_str(),"resources/stone.png");
	//addTexture(Avatar::NAME.c_str(),"resources/stone.png");
}

void assetmanager::addTexture(const char* name, const char* path) {
	std::shared_ptr<Texture> tex(new Texture());
	tex->loadFromFile(path);
	textureMap[name] = tex;
}

asset* assetmanager::get(const char* name) {
	if(name == Avatar::NAME.c_str() && numPlayer < 8) 
	{
		return new asset(textureMap[avatar_names[numPlayer++]]);
	}
	return new asset(textureMap[name]);
}