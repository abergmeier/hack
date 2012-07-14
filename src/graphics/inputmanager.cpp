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
	while (window->pollEvent(event))
	{
		if(event.type == sf::Event::GainedFocus) {
			focus = true;
		}
		if(event.type == sf::Event::LostFocus) {
			focus = false;
		}

		//when the application has the focus, all events will be handled
		if (focus) 
		{
			if (event.type == sf::Event::Closed) {
				window->close();
			}
			if (event.type == sf::Event::KeyPressed) {
				keys[event.key.code] = true;
			}
			if (event.type == sf::Event::KeyReleased) {
				keys[event.key.code] = false;
			}
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
	
	//movement
	int x,y;
	x = 0;
	y = 0;
	if(keys[Keyboard::W]) y -= 3;
	if(keys[Keyboard::S]) y += 3;
	if(keys[Keyboard::A]) x -= 3;
	if(keys[Keyboard::D]) x += 3;
	if((x != 0.0 || y != 0.0) && moveCallback) moveCallback(x,y);

	//attack
	if(keys[Keyboard::Space] && attackCallback) {
		attackCallback(true);
	} else if(!keys[Keyboard::Space] && attackCallback) {
		attackCallback(false);
	}

}

void inputmanager::registerCallbacks(std::function<void(int,int)> position,std::function<void(int,int)> rotation,std::function<void(bool)> attack) {
	moveCallback = position;
	rotateCallback = rotation;
	attackCallback = attack;
}