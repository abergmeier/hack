#pragma once
#ifndef _INPUTMANAGER_HPP_
#define _INPUTMANAGER_HPP_

#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>

class inputmanager
{
public:
	inputmanager(sf::RenderWindow* window);
	~inputmanager(void);
	void tick();
	//handle input self, only get callback functions
private:
	void (*moveCommand)(int x, int y);
	void (*attackCommand)();
	
	std::map<sf::Keyboard::Key,bool> keys;
	sf::RenderWindow* window;
};

#endif // !_INPUTMANAGER_HPP_




