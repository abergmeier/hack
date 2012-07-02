#include "renderer.hpp"
#include "assetmanager.hpp"
#include <iostream>
#include <map>
#include <string>
using namespace sf;
using namespace std;
using namespace hack::logic;

renderer::renderer(int width, int height)
	: window(new RenderWindow(sf::VideoMode(width, height), "hack game")), input(new inputmanager(window))
{
	window->setFramerateLimit(60);
}


renderer::~renderer(void)
{
	entities.clear();
}

void renderer::run() {
	sf::Text text("hack game");
	
    while (window->isOpen())
    {
		//clear screen
		window->clear();
        
		//handle inputs
		input->tick();
		
        //test text
        window->draw(text);
		
		//render all registered entities
		renderAll();

		//show everything rendered on screen
		window->display();
	}
}

void renderer::renderAll() {
	map<entity*,asset*>::iterator iter;
	for(iter = entities.begin(); iter != entities.end(); iter++) {
		iter->second->spr->setPosition((float)iter->first->getX(),(float)iter->first->getY());
		//TODO: richtige größen
		window->draw(*iter->second->spr);
	}
}

void renderer::registerEntity(entity& e, const char* typeName) {
	entities[&e] = am.get(typeName);
}


void renderer::deleteEntity(entity& e) {
	entities.erase(&e);
}