#include "assetmanager.hpp"
#include "../logic/stone.hpp"
#include "../logic/avatar.hpp"
#include "../logic/weapon.hpp"
#include <iostream>
#include <string>

using namespace sf;
using namespace hack::logic;
using namespace hack::graphics;

assetmanager::assetmanager(void)
	: numPlayer(0)
{
	init();
}


assetmanager::~assetmanager(void)
{
	textureMap.clear();
}

void assetmanager::init() {
	//initializing all 8 playercolors
	/*const char *player_colors[] = {"normal","red","blue","green","yellow","mono","brown","purple"};
	std::string p = Avatar::NAME; 
	std::string resource_start = "resources/avatar-";
	std::string resource_end = ".png";
	for(auto k : player_colors) {
		std::string temp = p+k;
		avatar_names.push_back(temp);
		addTexture(avatar_names.back().c_str(),(resource_start+k+resource_end).c_str());
		//std::cout << avatar_names.end()->c_str() << std::endl;
	}*/
	
	//player sprites
	avatar_names.push_back("avatarpurple");
	addTexture(avatar_names.back(),"resources/avatar-purple.png");

	avatar_names.push_back("avatarmono");
	addTexture(avatar_names.back(),"resources/avatar-mono.png");

	avatar_names.push_back("avatargreen");
	addTexture(avatar_names.back(),"resources/avatar-green.png");

	avatar_names.push_back("avatarred");
	addTexture(avatar_names.back(),"resources/avatar-red.png");

	avatar_names.push_back("avatarblue");
	addTexture(avatar_names.back(),"resources/avatar-blue.png");

	avatar_names.push_back("avatarnormal");
	addTexture(avatar_names.back(),"resources/avatar-normal.png");

	avatar_names.push_back("avataryellow");
	addTexture(avatar_names.back(),"resources/avatar-yellow.png");

	//fallback if too many players
	addTexture(Avatar::NAME.c_str(),"resources/stone.png");
	
	//stone texture
	addTexture(Stone::NAME.c_str(),"resources/stone.png");

	//weapon
	addTexture(Weapon::NAME.c_str(),"resources/weapon.png");
}

void assetmanager::addTexture(const char* name, const char* path) {
	std::shared_ptr<Texture> tex(new Texture());
	tex->loadFromFile(path);
	textureMap[name] = tex;
}

asset* assetmanager::get(const char* name) {
	if(name == Avatar::NAME.c_str() && numPlayer < 8) 
	{
		return new asset(textureMap[avatar_names.at(numPlayer++)]);
	}
	return new asset(textureMap[name]);
}