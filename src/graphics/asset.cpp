#include "asset.hpp"

using namespace sf;

asset::asset(Texture* tex) 
	: tex(tex), spr(new Sprite()), name("hallo")
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
	: tex(new Texture(*a.tex)), spr(new Sprite(*a.spr)), name(a.name)
{	
}