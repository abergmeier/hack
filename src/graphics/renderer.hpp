#pragma once
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "assetmanager.hpp"
#include "inputmanager.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <map>
#include "../logic/entity.hpp"
#include "asset.hpp"
#include <vector>

class renderer
{
public:
	renderer(int width, int height);
	virtual ~renderer(void);

	void run();
	void start();

	void registerEntity(hack::logic::entity& e, const char* typeName);
	void deleteEntity(hack::logic::entity& e);
private:
	sf::RenderWindow *window;
	inputmanager *input;
	std::map<hack::logic::entity*,asset*> entities;
	assetmanager am;
	
	void renderAll();
	bool isCloseRequest();
	void init();
};
#endif // !_RENDERER_HPP_



