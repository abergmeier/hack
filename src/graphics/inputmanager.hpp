#pragma once
#ifndef _INPUTMANAGER_HPP_
#define _INPUTMANAGER_HPP_

#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>
#include <functional>

class inputmanager
{
public:
	inputmanager(sf::RenderWindow* window);
	~inputmanager(void);
	void tick();
	
	void registerCallbacks(std::function<void(float,float)>,std::function<void(float)>,std::function<void()>);
private:
	std::function<void(float,float)> moveCallback;
	std::function<void(float)> rotateCallback;
	std::function<void()> attackCallback;
	
	std::map<sf::Keyboard::Key,bool> keys;
	sf::RenderWindow* window;
};

#endif // !_INPUTMANAGER_HPP_




