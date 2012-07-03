#pragma once
#include "../logic/entity.hpp"
#include <SFML\Graphics.hpp>

class asset
{
public:
	asset(sf::Texture* tex);
	~asset(void);
	asset(const asset& a);

	const sf::Texture* getTexture() const { return tex; }
	const sf::Sprite* getSprite() const { return spr; }
	
	char * name;
	sf::Texture* tex;
	sf::Sprite* spr;
private:
	
};