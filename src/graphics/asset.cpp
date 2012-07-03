#include "asset.hpp"

using namespace sf;

asset::asset(std::shared_ptr<Texture> texture) 
	: tex(texture), spr()
{
	spr.setTexture(*tex,true);
}

asset::asset(const asset& a) 
	: tex(a.tex), spr(a.spr)
{

}

asset::~asset(void)
{
}

void asset::setPosition(float x, float y) {
	spr.setPosition(x,y);
}
void asset::setRotation(float angle) {
	spr.setRotation(angle);
}
void asset::setSize(float width, float height) {
	spr.scale(width/(float)tex->getSize().x,height/(float)tex->getSize().y);
}