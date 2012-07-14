/**
	asset.hpp

	container for a sprite with a texture on it
*/
#pragma once
#ifndef _ASSET_HPP_
#define _ASSET_HPP_


#include "../logic/entity.hpp"
#include <SFML/Graphics.hpp>
#include <memory>

namespace hack {
namespace graphics {

/**
	asset container holding a sprite with a texture on it
	it also has methods to manipulate the sprites position, rotation and size
*/
class asset
{
public:
	//constructor, destructor, copy constructor and assignment operator
	asset(std::shared_ptr<sf::Texture> texture);
	~asset(void);
	asset(const asset& a);
	asset& operator=(const asset& a);

	//const getter for the texture and sprite to be able to display it
	const sf::Texture& getTexture() const { return *tex; }
	const sf::Sprite& getSprite() const { return spr; }

	//manipulation functions
	void setPosition(float,float);
	void setRotation(float);
	void setSize(float,float);

private:
	//shared pointer for the texture (texture is only loaded once and used in all assets with the same type)
	std::shared_ptr<sf::Texture> tex;
	
	//sprite holding the texture and defining its position, size and rotation
	sf::Sprite spr;
};

}
}

#endif // !_ASSET_HPP_