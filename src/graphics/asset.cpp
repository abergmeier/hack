#include "asset.hpp"

using namespace sf;

asset::asset(Texture* tex) 
	: name("hallo"), tex(tex), spr(new Sprite())
{
	spr->setTexture(*tex,true);
}

asset::~asset(void)
{
	Texture* t;
	t = new Texture();
	Texture k(*t);
}

asset::asset(const asset& a)
	: name(a.name), tex(new Texture(*a.tex)), spr(new Sprite(*a.spr))
{	
}
