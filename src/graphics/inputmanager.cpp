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
		if (event.type == sf::Event::Closed)
			window->close();
		if (event.type == sf::Event::KeyPressed) {
			cout << event.key.code << endl;
		}
	}
}