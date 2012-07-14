#include "asset.hpp"

using namespace sf;
using namespace hack::graphics;

asset::asset(std::shared_ptr<Texture> texture) 
	: tex(texture), spr()
{
	//set texture visual
	spr.setTexture(*tex,true);
	//set new origin for rotation
	spr.setOrigin(static_cast<float>(tex->getSize().x)/2,static_cast<float>(tex->getSize().y)/2);
}

asset::asset(const asset& a)
	: tex(a.tex), spr(a.spr)
{
}

asset& asset::operator=(const asset& a) {
	tex = a.tex;
	spr = a.spr;
	return *this;
}

asset::~asset(void)
{
}

void asset::setPosition(float x, float y) {
	//set position of sprite
	spr.setPosition(x,y);
}
void asset::setRotation(float angle) {
	//set rotation / facing angle of the sprite
	spr.setRotation(angle);
}
void asset::setSize(float width, float height) {
	//set new size
	spr.scale(width/static_cast<float>(tex->getSize().x),height/static_cast<float>(tex->getSize().y));
	//set new origin for rotation
	spr.setOrigin(static_cast<float>(tex->getSize().x)/2,static_cast<float>(tex->getSize().y)/2);
}