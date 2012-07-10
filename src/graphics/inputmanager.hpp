#pragma once
#ifndef _INPUTMANAGER_HPP_
#define _INPUTMANAGER_HPP_

#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>
#include <functional>

namespace hack {
namespace graphics {

class inputmanager
{
public:
	inputmanager(sf::RenderWindow* window);
	~inputmanager(void);
	
	//handles given input for 1 rendertick
	void tick();
	
	void registerCallbacks(std::function<void(int,int)>,std::function<void(int,int)>,std::function<void()>);
private:
	std::function<void(int,int)> moveCallback;
	std::function<void(int,int)> rotateCallback;
	std::function<void()> attackCallback;

	void handleKeys();

	sf::RenderWindow* window;
	std::map<sf::Keyboard::Key,bool> keys;
	bool focus;
};

}
}

#endif // !_INPUTMANAGER_HPP_




