/**
	renderer.hpp

	renderer is used to render the scene in a loop of 60 frames per second
	it also holds the inputmanager and the assetmanager.
	you can register and remove entities from the renderer, which will create and destroy assets.
	those assets will be drawn at the position, rotation and with the size of their registered entities.
*/
#pragma once
#ifndef _RENDERER_HPP_
#define _RENDERER_HPP_

#include "assetmanager.hpp"
#include "inputmanager.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <map>
#include <memory>
#include "../logic/entity.hpp"
#include "asset.hpp"
#include <vector>

namespace hack {
namespace graphics {

/**
	renderer class with holds all drawable assets and renders them to the screen
*/
class renderer
{
public:
	//typedef of a weak pointer of entities to value_type
	typedef std::weak_ptr<hack::logic::entity> value_type;

	//constructor (with width and height of the window) and destructor
	renderer(int width, int height);
	virtual ~renderer(void);

	//starts the renderer and displays all registered assets/entities with 60 frames per second
	void run();

	//function for registration of entities, it also creates an asset based on their name
	void insert(const value_type& e);

	//destruction of an entity
	void erase( const value_type& e);

	//getter to the inputmanager, so that callbacks can be registered
	inputmanager& getInputmanager() { return *input; };
private:
	//pointer to renderwindow (for displaying stuff)
	sf::RenderWindow *window;

	//pointer to inputmanager (for checking inputs each frame)
	inputmanager *input;

	//map of entites and their assigned assets
	std::map<hack::logic::entity*, std::unique_ptr<asset>> entities;

	//assetmanager, so that assets can be created
	assetmanager am;
	
	//renders all entities/assets
	void renderAll();
};

}
}
#endif // !_RENDERER_HPP_



