#pragma once
#include "../logic/entity.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

class asset
{
public:
	asset(std::shared_ptr<sf::Texture> texture);
	~asset(void);
	asset(const asset& a);
	asset& operator=(const asset& a);

	const sf::Texture& getTexture() const { return *tex; }
	const sf::Sprite& getSprite() const { return spr; }

	void setPosition(float,float);
	void setRotation(float);
	void setSize(float,float);

private:
	std::shared_ptr<sf::Texture> tex;
	sf::Sprite spr;
};
