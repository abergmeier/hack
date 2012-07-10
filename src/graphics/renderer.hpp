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
	bool isCloseRequest();
	void init();
};
#endif // !_RENDERER_HPP_



