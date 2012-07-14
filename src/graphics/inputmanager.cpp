#include "inputmanager.hpp"

using namespace sf;
using namespace std;
using namespace hack::graphics;

inputmanager::inputmanager(RenderWindow* window)
	: window(window), focus(true)
{
}


inputmanager::~inputmanager(void)
{
}


void inputmanager::tick() {
	sf::Event event;

	//pull all queued events out of the renderwindow
	while (window->pollEvent(event))
	{
		//check if window is focussed
		if(event.type == sf::Event::GainedFocus) {
			focus = true;
		}
		if(event.type == sf::Event::LostFocus) {
			focus = false;
		}

		//when the application has the focus, all events will be handled
		if (focus) 
		{
			//window closed
			if (event.type == sf::Event::Closed) {
				window->close();
			}
			//save all pressed keys
			if (event.type == sf::Event::KeyPressed) {
				keys[event.key.code] = true;
			}
			//set all released keys to false
			if (event.type == sf::Event::KeyReleased) {
				keys[event.key.code] = false;
			}
			//call callback for mouse movement if one is set
			if (event.type == sf::Event::MouseMoved) {
				//rotation
				if(rotateCallback) rotateCallback(event.mouseMove.x,event.mouseMove.y);
			}
		}
	}
	//handle key inputs, after they where accumulated
	handleKeys();
}

void inputmanager::handleKeys() {
	//close game on escape
	if(keys[Keyboard::Escape]) {
		window->close();
		return;
	}
	
	//keyboard movement
	int x,y;
	x = 0;
	y = 0;
	if(keys[Keyboard::W]) y -= 3;
	if(keys[Keyboard::S]) y += 3;
	if(keys[Keyboard::A]) x -= 3;
	if(keys[Keyboard::D]) x += 3;
	if((x != 0.0 || y != 0.0) && moveCallback) moveCallback(x,y);

	//attack on space key, also execute the attack callback with false to retract the weapon
	if(keys[Keyboard::Space] && attackCallback) {
		attackCallback(true);
	} else if(!keys[Keyboard::Space] && attackCallback) {
		attackCallback(false);
	}

}

void inputmanager::registerCallbacks(std::function<void(int,int)> position,std::function<void(int,int)> rotation,std::function<void(bool)> attack) {
	//set all callback functions
	moveCallback = position;
	rotateCallback = rotation;
	attackCallback = attack;
}