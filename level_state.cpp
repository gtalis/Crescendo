#include "level_state.hpp"

#include <iomanip>
#include <sstream>
#include <fstream>

LevelState::LevelState(int level, bool skipIntro) : level(level) {
	if (skipIntro) {
		gameState = 1;
	}
	else {
		gameState = 0;
	}
}

LevelState::~LevelState() {
	for (Lever *lever : levers) {
		delete lever;
	}

	for (BeatBlock *block : blocks) {
		delete block;
	}

	for (sf::Music *mus : music) {
		delete mus;
	}
}

void LevelState::init() {
	elevator.setTexture(loadTexture("img/elevator.png"));
	elevator.setTextureRect(sf::IntRect(0, 0, 40, 135));

	floorDisplay.setTexture(loadTexture("img/font.png"));
	floorDisplay.setPosition(13, 98);
	floorDisplay.setColor(sf::Color(127, 0, 0));
	std::stringstream ss;
	ss << "lv" << level;
	floorDisplay.setText(ss.str());
	ss.str("");

	ss << "level/level" << level << ".png";
	levelSprite.setTexture(loadTexture(ss.str()));
	ss.str("");
	levelSprite.setPosition(40, 0);

	ss << "level/level" << level << "m.png";
	levelMask.loadFromFile(ss.str());
	ss.str("");

	player.setState(this);
	player.init();

	hud.setTexture(loadTexture("img/hud.png"));
	hud.setTextureRect(sf::IntRect(0, 0, 36, 91));
	ambientBar.setTexture(loadTexture("img/hud.png"));
	personalBar.setTexture(loadTexture("img/hud.png"));
	alertOverlay.setTexture(loadTexture("img/alert.png"));

	elevatorSound.setBuffer(loadSoundBuffer("snd/elevator.wav"));
	doorSound.setBuffer(loadSoundBuffer("snd/door.wav"));
	alertSound.setBuffer(loadSoundBuffer("snd/alert.wav"));

	for (int i = 0; i < 3; i++) {
		music.push_back(new sf::Music());
		ss << "mus/level" << level << "-" << i + 1 << ".ogg";
		music[i]->openFromFile(ss.str());
		ss.str("");
		music[i]->setVolume(0);
		music[i]->setLoop(true);
	}
	music[0]->setVolume(100);

	if (gameState == 1) {
		for (sf::Music *mus : music) {
			mus->play();
		}
		elevator.setTextureRect(sf::IntRect(40, 0, 40, 135));
		// Todo: fix player making noise when intro is skipped
	}

	setupLevel();
}

void LevelState::gotEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::X && gameState == 1) {
			for (Lever *lever : levers) {
				if (!lever->isFlipped() && lever->isOverlapping(player.getCenterPosition())) {
					lever->flip();
					setSection(section + 1);
				}
			}
		}
		else if (event.key.code == sf::Keyboard::Escape) {
			// Todo: pause menu
		}

		// Debug Keys
		if (event.key.code == sf::Keyboard::Num1) {
			setSection(1);
		}
		else if (event.key.code == sf::Keyboard::Num2) {
			setSection(2);
		}
		else if (event.key.code == sf::Keyboard::Num3) {
			setSection(3);
		}
		else if (event.key.code == sf::Keyboard::Num9) {
			if (level > 1) {
				game->changeState(new LevelState(level - 1));
			}
		}
		else if (event.key.code == sf::Keyboard::Num0) {
			game->changeState(new LevelState(level + 1));
		}
	}
}

void LevelState::update(sf::Time elapsed) {
	if (gameState == 0) {
		gameTimer -= elapsed.asSeconds();
		if (gameTimer <= 1.5 && elevatorSound.getStatus() == sf::Sound::Stopped) {
			elevatorSound.play();
		}
		if (gameTimer <= 0) {
			gameState = 1;
			for (sf::Music *mus : music) {
				mus->play();
			}
			elevator.setTextureRect(sf::IntRect(40, 0, 40, 135));
			doorSound.play();
		}
	}
	if (gameState == 1) {
		// Tick variables
		beatCounter += elapsed.asSeconds();
		if (beatCounter >= 60.f / bpm) {
			beatCounter -= 60.f / bpm;
			beat++;
		}
		volume *= std::pow(.5, elapsed.asSeconds());

		// Calculate music volume
		calculateVolume();

		// Update player
		player.update(elapsed);

		// Calculate alert
		if (player.getVolume() <= volume && alert > 0) {
			alert -= elapsed.asSeconds();
			if (alert < 0) {
				alert = 0;
			}
		}
		else if (player.getVolume() > volume) {
			alert += elapsed.asSeconds();
		}
		if (alert >= .1) {
			gameState = 2;
			gameTimer = 1.5;
			for (sf::Music *mus : music) {
				mus->stop();
			}
			alertSound.play();
		}

		// Update hud
		if (volume > 0) {
			float tempVolume = volume;
			if (tempVolume > 4) {
				tempVolume = 4;
			}
			int height = 69 * tempVolume / 4;
			ambientBar.setPosition(8, 18 + (69 - height));
			ambientBar.setTextureRect(sf::IntRect(36, 69 - height, 11, height));
		}
		if (player.getVolume() > 0) {
			float tempVolume = player.getVolume();
			if (tempVolume > 4) {
				tempVolume = 4;
			}
			int height = 69 * tempVolume / 4;
			personalBar.setPosition(21, 18 + (69 - height));
			personalBar.setTextureRect(sf::IntRect(47, 69 - height, 11, height));
		}
	}
	if (gameState == 2) {
		gameTimer -= elapsed.asSeconds();
		if (gameTimer <= 0) {
			game->changeState(new LevelState(level, 1));
		}
	}
}

void LevelState::render(sf::RenderWindow &window) {
	window.draw(elevator);
	if (gameState == 0) {
		if (gameTimer < 1.5) {
			window.draw(floorDisplay);
		}
	}
	else {
		if (beat % 4 <= 2) {
			window.draw(floorDisplay);
		}
		window.draw(levelSprite);

		for (Lever *lever : levers) {
			window.draw(*lever);
		}

		for (BeatBlock *block : blocks) {
			window.draw(*block);
		}

		window.draw(player);

		window.draw(hud);
		if (volume > 0) {
			window.draw(ambientBar);
		}
		if (player.getVolume()) {
			window.draw(personalBar);
		}

		if (gameState == 2) {
			window.draw(alertOverlay);
		}
	}
}

bool LevelState::checkCollision(sf::Vector2f position, int width, int height) {
	if (position.y + height > 125) {
		return true;
	}
	else if (position.y < 10) {
		return true;
	}
	else if (position.x + width > 230) {
		return true;
	}
	else if (position.x < 10) {
		return true;
	}
	else if (position.x < 40 && position.y < 95) {
		return true;
	}
	else if (position.x > 40 && position.x + width < 240 && position.y > 0 && position.y + height < 135) {
		if (levelMask.getPixel(position.x - 40, position.y).b == 255) {
			return true;
		}
		else if (levelMask.getPixel(position.x - 40 + width, position.y).b == 255) {
			return true;
		}
		else if (levelMask.getPixel(position.x - 40, position.y + height / 2).b == 255) {
			return true;
		}
		else if (levelMask.getPixel(position.x - 40 + width, position.y + height / 2).b == 255) {
			return true;
		}
		else if (levelMask.getPixel(position.x - 40, position.y + height).b == 255) {
			return true;
		}
		else if (levelMask.getPixel(position.x - 40 + width, position.y + height).b == 255) {
			return true;
		}
	}
	return false;
}

bool LevelState::isMetal(sf::Vector2f position) {
	if (position.x < 40 || position.x > 240 || position.y < 0 || position.y > 135) {
		return false;
	}
	else {
		return levelMask.getPixel(position.x - 40, position.y).g == 255;
	}
}

void LevelState::setupLevel() {
	if (level == 1) {
		levers.push_back(new Lever(this, sf::Vector2f(90, 117)));
	}
}

void LevelState::calculateVolume() {
	if (level == 1) {
		if (beatCounter < .15) {
			int localBeat = beat % 16;
			if (section >= 1) {
				if ((localBeat % 2 == 0 && localBeat != 14) || localBeat == 9) {
					setVolume(1.8);
				}
			}
			if (section >= 2) {
				if ((localBeat % 2 == 0 && localBeat != 14) || localBeat == 11 || beat % 32 == 25) {
					setVolume(2.5);
				}
			}
			if (section == 3) {
				if (localBeat % 2 == 0 && localBeat < 8) {
					setVolume(3.4);
				}
			}
		}
	}
}

void LevelState::setVolume(float volume) {
	if (volume > this->volume) {
		this->volume = volume;
	}
}

void LevelState::setSection(int section) {
	music[this->section - 1]->setVolume(0);
	music[section - 1]->setVolume(100);
	this->section = section;
}

