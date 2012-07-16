/**
	inputmanager.hpp

	manages keyboard inputs.
	also functions can be registered to be executed on move, attack and rotation inputs
*/

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
	//constructor (with RenderWindow as parameter to get the current input events) and destructor
	inputmanager(sf::RenderWindow* window);
	~inputmanager(void);
	
	//this function registers which the inputs are used for further processing
	//must be called once per frame
	void tick();
	
	//registration function to register callback for further character manipulation
	//these functions must be delivered by the logic or else they wont be executed
	void registerCallbacks(std::function<void(int,int)>,std::function<void(int,int)>,std::function<void(bool)>);
private:
	//variables holding the function callbacks
	std::function<void(int,int)> moveCallback;
	std::function<void(int,int)> rotateCallback;
	std::function<void(bool)> attackCallback;

	//processes the inputs and executes their callbacks if they are registered
	void handleKeys();

	//pointer to the renderwindow
	sf::RenderWindow* window;

	//map that holds all keyboard inputs per tick
	std::map<sf::Keyboard::Key,bool> keys;
	
	//variable that is set true, if the renderwindow is focussed
	//if it is not focussed, this will prevent inputs from being processed
	bool focus;

	//for checking purpose, that missing keyinput wont retract the attack even though the mousebutton is attacking
	bool attacking;
};

}
}

#endif // !_INPUTMANAGER_HPP_




