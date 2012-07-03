#include "inputmanager.hpp"

using namespace sf;
using namespace std;

inputmanager::inputmanager(RenderWindow* window)
	: window(window)
{
}


inputmanager::~inputmanager(void)
{
}


void inputmanager::tick() {
	sf::Event event;
	while (window->pollEvent(event))
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
	if(keys[Keyboard::Space] && attackCallback) attackCallback();
}

void inputmanager::registerCallbacks(std::function<void(int,int)> position,std::function<void(int,int)> rotation,std::function<void()> attack) {
	moveCallback = position;
	rotateCallback = rotation;
	attackCallback = attack;
}