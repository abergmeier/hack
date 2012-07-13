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

class renderer
{
public:
	typedef std::weak_ptr<hack::logic::entity> value_type;
	renderer(int width, int height);
	virtual ~renderer(void);

	void run();

	void insert(const value_type& e);
	void erase( const value_type& e);
	inputmanager& getInputmanager() { return *input; };
private:
	sf::RenderWindow *window;
	inputmanager *input;
	std::map<hack::logic::entity*,asset*> entities;
	assetmanager am;
	
	void renderAll();
	void init();
};

}
}
#endif // !_RENDERER_HPP_



