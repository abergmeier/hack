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
	avatar_names.clear();
}

void assetmanager::init() {
	//registration of different player textures
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

	//fallback texture if too many players
	addTexture(Avatar::NAME, "resources/stone.png");
	
	//stone texture
	addTexture(Stone::NAME, "resources/stone.png");

	//weapon texture
	addTexture(Weapon::NAME, "resources/weapon.png");
}

void assetmanager::addTexture(const std::string& name, const std::string& path) {
	auto tex = std::make_shared<Texture>();
	tex->loadFromFile(path);
	textureMap[name] = tex;
}

asset assetmanager::get(const std::string& name) {
	//if a avatar texture is asked for, return on of the colored of, if not or too many players registered return
	//the designated texture of the name given
	if(name == Avatar::NAME && numPlayer < 8)
		return asset(textureMap[avatar_names.at(numPlayer++)]);

	return asset(textureMap[name]);
}
