#include "game.hpp"

#include "state.hpp"
#include "menu_state.hpp"
#include <iostream>
#include <algorithm>

#ifdef __linux__
#include <stdlib.h> // getenv
#include <sys/stat.h>
#endif

Game::Game() {
	// Create game window
	//window = new sf::RenderWindow(sf::VideoMode(1920, 1080), "Crescendo", sf::Style::Fullscreen);
	window = new sf::RenderWindow(sf::VideoMode(960, 540), "Crescendo");
	window->setFramerateLimit(60);
	fullscreen = false;
	scale = std::min(window->getSize().x / 240, window->getSize().y / 135);

	// Initialize game-wide variables
#ifdef __linux__
	getResourceDir();
#endif

	// Initial state
	changeState(new MenuState());

	// Start the clock
	sf::Clock clock;

	while (window->isOpen()) {
		// State
		if (nextState) {
			// Load up the new state
			if (state)
				delete state;
			state = nextState;
			nextState = nullptr;
			state->setGame(this);
			state->init();

			// Restart the clock to account for load time
			clock.restart();
		}

		// Event
		sf::Event event;
		while (window->pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				exit();
			}
			else if (event.type == sf::Event::Resized) {
				window->setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::F11) {
					if (fullscreen) {
						delete window;
						window = new sf::RenderWindow(sf::VideoMode(960, 540), "Crescendo");
						window->setFramerateLimit(60);
						fullscreen = false;
					}
					else {
						delete window;
						window = new sf::RenderWindow(sf::VideoMode(1920, 1080), "Crescendo", sf::Style::Fullscreen);
						window->setFramerateLimit(60);
						fullscreen = true;
					}
				}
			}

			if (state) {
				state->gotEvent(event);
			}
		}

		// Update
		sf::Time elapsed = clock.restart();
		// If the game freezes, make sure no time passes
		if (elapsed.asMilliseconds() >= 250) {
			elapsed = sf::Time::Zero;
		}
		if (state)
			state->update(elapsed);

		// Render
		window->clear();
		if (state) {
			state->render(*window);
		}

		// Pixelate
		sf::Texture postTexture;
		postTexture.create(window->getSize().x, window->getSize().y);
		postTexture.update(*window);

		sf::Sprite postSprite(postTexture, sf::IntRect(0, 0, 240, 135));
		scale = std::min(window->getSize().x / 240, window->getSize().y / 135);
		postSprite.setScale(scale, scale);
		window->draw(postSprite);

		window->display();

		// Exit
		if (exiting) {
			window->close();
		}
	}
}


Game::~Game() {
	if (state)
		delete state;
	if (nextState)
		delete nextState;

	delete window;
}

sf::RenderWindow *Game::getWindow() {
	return window;
}

void Game::changeState(State *newState) {
	if (nextState)
		delete nextState;
	nextState = newState;
}

void Game::exit() {
	exiting = true;
}

#ifdef __linux__
const std::string data_install_dir = "/Crescendo/";
const std::string img_install_dir = "img/";
void Game::getResourceDir()
{
	// Installation process should install resources
	// in one of the directories defined by the XDG standard, as follows
	// /path/to/xdg_data_dir/Compactory/Resource

	// Get list of data dirs as defined by XDG standard
	std::string data_dirs = getenv("XDG_DATA_DIRS");
	std::size_t found = data_dirs.find_first_of(":");
	std::size_t tmp = 0;

	struct stat buffer;

	// Cycle through each of them until right one is found
	while (found != std::string::npos)
	{
		std::string tmpDir = data_dirs.substr(tmp, found-tmp);
		std::string resDir = tmpDir + data_install_dir + img_install_dir;

	    if (stat (resDir.c_str(), &buffer) == 0) {
			// Directory was found.
			// It will be prepended to the texture filename
			rsrcDir = tmpDir + data_install_dir;
			return;
	    }

	    // Continue searching
		tmp = found + 1;
		found = data_dirs.find_first_of(":", found+1);
	}
}
#endif

sf::Texture& Game::loadTexture(std::string filename) {
#ifdef __linux__
	// Prepend directory location where resources are stored
	// such as "img/orb.png" becomes "/path/to/rsrc_dir/img/orb.png"
	filename = rsrcDir + filename;
#endif

	if (textures.count(filename) == 0) {
		sf::Texture newTexture;
		if (newTexture.loadFromFile(filename)) {
			textures.insert(std::pair<std::string, sf::Texture>(filename, newTexture));
			// Debug
			//std::cout << "New texture created (" << filename << ")\n";
		}
		else {
			if (textures.count("BadTexture") == 0) {
				sf::Image badImage;
				badImage.create(2, 2, sf::Color::Black);
				badImage.setPixel(0, 0, sf::Color::Cyan);
				badImage.setPixel(1, 1, sf::Color::Cyan);
				newTexture.loadFromImage(badImage);
				newTexture.setRepeated(true);
				textures.insert(std::pair<std::string, sf::Texture>("BadTexture", newTexture));

				// Debug
				//std::cout << "Bad texture created (" << filename << ")\n";
			}
			else {
				// Debug
				//std::cout << "Bad texture loaded (" << filename << ")\n";
			}
			return textures["BadTexture"];
		}
	}
	else {
		// Debug
		//std::cout << "Existing texture loaded (" << filename << ")\n";
	}
	return textures[filename];
}

sf::SoundBuffer& Game::loadSoundBuffer(std::string filename) {
#ifdef __linux__
	// Prepend directory location where resources are stored
	// such as "mus/levelxxx.ogg" becomes "/path/to/rsrc_dir/mus/levelxxx.ogg"
	filename = rsrcDir + filename;
	std::cout << "filename is " << filename << std::endl;
#endif

	if (soundBuffers.count(filename) == 0) {
		sf::SoundBuffer newSoundBuffer;
		if (newSoundBuffer.loadFromFile(filename)) {
			soundBuffers.insert(std::pair<std::string, sf::SoundBuffer>(filename, newSoundBuffer));
			// Debug
			//std::cout << "New sound buffer created (" << filename << ")\n";
		}
		else {
			// Debug
			//std::cout << "Cannot create sound buffer (" << filename << ")\n";
		}
	}
	else {
		// Debug
		//std::cout << "Existing sound buffer loaded (" << filename << ")\n";
	}
	return soundBuffers[filename];
}

sf::Vector2u Game::screenSize() {
	return window->getSize();
}
