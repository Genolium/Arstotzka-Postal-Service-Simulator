#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <random>
#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <vector>

#include <cstdio>

#pragma execution_character_set("utf-8")

enum class StampType {
	Circle,
	Rectangle,
	Sprite
};

struct Letter {
	std::string city;
	StampType stampType;
	sf::Shape* shape;
	sf::Sprite* sprite;
	sf::Vector2f position;
};

class Game {
public:
	Game() {
		window.create(sf::VideoMode(1600, 1200), "Arstotzka Postal Service Simulator");
		font.loadFromFile("font.ttf");

		if (back.loadFromFile("background.png"))
		{
			backgroundSprite.setTexture(back);
			// Растягиваем спрайт на весь экран
			sf::Vector2u windowSize = window.getSize();
			sf::Vector2f spriteScale(static_cast<float>(windowSize.x) / back.getSize().x,
				static_cast<float>(windowSize.y) / back.getSize().y);
			backgroundSprite.setScale(spriteScale);
		}
		else
		{
			// Обработка ошибки загрузки текстуры
			std::cout << "Failed to load texture: back" << std::endl;
		}

		// Загрузка текстур из файлов
		for (const auto& file : textureFiles)
		{
			sf::Texture texture;
			if (texture.loadFromFile(file))
			{
				textures.push_back(texture);
			}
			else
			{
				// Обработка ошибки загрузки текстуры
				std::cout << "Failed to load texture: " << file << std::endl;
			}
		}

		for (const auto& sound : soundFiles) {
			auto music = std::make_unique<sf::Music>();
			if (music->openFromFile(sound)) {
				sounds.push_back(std::move(music));
			}
			else {
				// Обработка ошибки загрузки музыки
				std::cout << "Failed to load sound: " << sound << std::endl;
			}
		}
	}



	void run() {
		while (window.isOpen()) {
			//интро
			showIntro();
			//игра
			for (int i = 1; i <= 7; i++) {
				if (!playDay(i)) {
					showOutro(false);
					break;
				}
			}
			//аутро
			showOutro(true);
		}
	}

private:
	void showIntro() {
		muteMusic();
		(*sounds[4]).setVolume(100);
		(*sounds[4]).setPitch(0.8);
		(*sounds[4]).setLoop(true);
		(*sounds[4]).play();
		window.clear(sf::Color::Black);
		std::string text = "Конгратюлатьйон май френд! Вы получили работу в Arstotzka Postal Service!\n\n\
Ваш task - сортировать the letters в соответствии с reference.\n\
Вы также ограничены продолжительностью дня, имейте это в виду, товарищ!\n\n\
Управление: Левая кнопка мыши - допустить письмо, \nправая кнопка мыши - отказать в отправке письма\n\n\n\
Чтобы запустить игру - нажмите E\n\n\
Автор игры - Иль Васюнин";
		sf::Text intro(sf::String::fromUtf8(text.begin(), text.end()), font, 40);
		intro.setPosition(100, 200);

		bool introShown = true;
		while (introShown) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::KeyPressed) {
					if (event.key.code == sf::Keyboard::E) {
						introShown = false;
					}
				}
				if (event.type == sf::Event::Closed) {
					exit();
				}
			}
			window.clear(sf::Color::Black);
			window.draw(intro);
			window.display();
		}
		(*sounds[0]).play();
	}

	void showOutro(bool hasWinned) {
		muteMusic();
		(*sounds[4]).stop();
		(*sounds[5]).stop();
		(*sounds[3]).stop();

		window.clear(sf::Color::Black);
		std::string text;

		if (hasWinned) {
			(*sounds[5]).setVolume(100);
			(*sounds[5]).setPitch(0.8);
			(*sounds[5]).play();
			text = "Поздравляю, товарищ, вы прекрасно выполнили свою работу! \n\n\
Партия направляет вам приглашение на работу таможенником!\nЧтобы покинуть игру, нажмите Е\n\nДля начала новой игры нажмите R";
		}
		else {
			(*sounds[3]).setVolume(100);
			(*sounds[3]).setPitch(0.8);
			(*sounds[3]).play();
			text = "Вы допустили слишком много ошибок и будете люстрированы.\n\n\
С завтрашнего дня вы будете переведены в санаторий!\n\
Чтобы покинуть игру, нажмите Е\n\nДля начала новой игры нажмите R";
		}
		sf::Text outro(sf::String::fromUtf8(text.begin(), text.end()), font, 40);
		outro.setPosition(100, 200);

		bool outroShown = true;
		while (outroShown) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::KeyPressed) {
					if (event.key.code == sf::Keyboard::E) {
						outroShown = false;
						exit();
						return;
					}
					else if (event.key.code == sf::Keyboard::R) {
						outroShown = false;
						// Сбрасываем все переменные
						letters.clear();
						references.clear();
						score = 0;
						mistakes = 0;

						// Перезапускаем игру
						run();
					}
				}
				if (event.type == sf::Event::Closed) {
					exit();
				}
			}
			window.clear(sf::Color::Black);
			window.draw(outro);
			window.display();
		}
	}

	//возвращает, продержался игрок или нет
	bool playDay(int day) {
		(*sounds[0]).setVolume(100);
		(*sounds[1]).setVolume(100);
		(*sounds[2]).setVolume(100);

		//вычисляем продолжительность дня в секундах
		int dayDuration = 10 + (day * 5);
		sf::Clock clock;

		//отрисовка счётчика дней
		sf::Text dayCounter("Day: " + std::to_string(day), font, 40);
		dayCounter.setPosition(100, 50);

		std::srand(std::time(nullptr));

		int numLetters = day + 1;
		int numReferences = day;
		int numCorrectLetters = rand() % (numLetters - 1) + 1;

		letters = generateLetters(numLetters);
		references = generateLetters(numReferences);
		std::vector<Letter> correctLetters = generateLetters(numCorrectLetters);
		std::copy(correctLetters.begin(), correctLetters.end(), std::back_inserter(letters));
		std::copy(correctLetters.begin(), correctLetters.end(), std::back_inserter(references));

		std::shuffle(letters.begin(), letters.end(), std::mt19937(rd()));
		std::shuffle(references.begin(), references.end(), std::mt19937(rd()));

		while (letters.size() > 11) {
			letters.resize(11);
		}
		while (references.size() > 11) {
			references.resize(11);
		}

		//задаем позиции писем
		for (int i = 0; i < letters.size(); i++) {
			letters[i].position = sf::Vector2f(100, 100 + i * 100);
		}

		int clickedLetters = 0;
		std::vector<bool> letterClicked(letters.size(), false);

		// Отображение писем и справочника
		while (clickedLetters < letters.size()) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::MouseButtonPressed) {
					for (int i = 0; i < letters.size(); i++) {
						if (!letterClicked[i]) {
							sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
							if (letters[i].shape && letters[i].shape->getGlobalBounds().contains(mousePos) ||
								letters[i].sprite && letters[i].sprite->getGlobalBounds().contains(mousePos)) {
								processClick(event.mouseButton.button, letters[i], letterClicked, i, clickedLetters, score);
								if (mistakes > 2) {
									return false;
								}
							}
						}
					}
				}
				if (event.type == sf::Event::Closed) {
					exit();
				}
			}

			if (clock.getElapsedTime().asSeconds() >= dayDuration) {
				return false;
			}

			//очищаем экран и отрисовываем день
			window.clear(sf::Color::Black);
			window.draw(backgroundSprite);
			window.draw(dayCounter);

			// Отрисовка надписи "Справочник"
			std::string refText = "Справочник";
			sf::Text referenceText(sf::String::fromUtf8(refText.begin(), refText.end()), font, 30);
			referenceText.setPosition(window.getSize().x - 400, 50);
			window.draw(referenceText);

			drawReference();
			for (int i = 0; i < letters.size(); i++) {
				if (!letterClicked[i]) {
					drawLetter(letters[i]);
				}
			}
			drawScore();

			// Отображаем таймер
			int remainingTime = dayDuration - static_cast<int>(clock.getElapsedTime().asSeconds());
			sf::Text timerText("Time: " + std::to_string(remainingTime), font, 25);
			timerText.setPosition(window.getSize().x - 200, window.getSize().y - 75);
			window.draw(timerText);

			window.display();
		}

		return true;
	}

	void drawReference() {
		int x = 1300;
		int y = 100;
		for (const auto& city : references) {
			sf::Text cityText(city.city, font, 25);
			cityText.setPosition(x, y);
			window.draw(cityText);
			y += 100;
		}

		x = 1200;
		y = 100;
		for (const auto& stamp : references) {
			if (stamp.shape) {
				stamp.shape->setPosition(x, y);
				window.draw(*stamp.shape);
			}
			else {
				stamp.sprite->setPosition(x, y);
				window.draw(*stamp.sprite);
			}
			y += 100;
		}
	}

	std::vector<Letter> generateLetters(int amount) {
		std::vector<Letter> generatedLetters;
		// Создание писем
		for (int i = 1; i <= amount; i++) {
			Letter letter = createStamp();
			letter.city = cities[rand() % cities.size()];
			generatedLetters.push_back(letter);
		}
		return generatedLetters;
	}

	Letter createStamp() {
		Letter letter;

		//потом поменять на 3
		letter.stampType = static_cast<StampType>(rand() % 3);

		switch (letter.stampType) {
		case StampType::Circle:
			letter.shape = new sf::CircleShape(40);
			letter.sprite = nullptr;
			break;
		case StampType::Rectangle:
			letter.shape = new sf::RectangleShape(sf::Vector2f(80, 80));
			letter.sprite = nullptr;
			break;
		case StampType::Sprite:
			letter.shape = nullptr;
			letter.sprite = new sf::Sprite(textures[std::rand() % textures.size()]);
			letter.sprite->setScale(2, 2);
			break;
		}

		if (letter.shape != nullptr) {
			letter.shape->setFillColor(colors[rand() % colors.size()]);
		}

		return letter;
	}

	bool isValidLetter(const Letter& letter) {
		for (const Letter& ref : references) {
			if (letter.city == ref.city && letter.stampType == ref.stampType) {
				if (letter.stampType == StampType::Circle && ref.stampType == StampType::Circle) {
					return static_cast<sf::CircleShape*>(letter.shape)->getRadius() == static_cast<sf::CircleShape*>(ref.shape)->getRadius() &&
						static_cast<sf::CircleShape*>(letter.shape)->getFillColor() == static_cast<sf::CircleShape*>(ref.shape)->getFillColor();
				}
				else if (letter.stampType == StampType::Rectangle && ref.stampType == StampType::Rectangle) {
					return static_cast<sf::RectangleShape*>(letter.shape)->getSize() == static_cast<sf::RectangleShape*>(ref.shape)->getSize() &&
						static_cast<sf::RectangleShape*>(letter.shape)->getFillColor() == static_cast<sf::RectangleShape*>(ref.shape)->getFillColor();
				}
				else if (letter.stampType == StampType::Sprite && ref.stampType == StampType::Sprite) {
					return letter.sprite->getTexture() == ref.sprite->getTexture();
				}
			}
		}
		return false;
	}

	void processClick(sf::Mouse::Button button, Letter& letter, std::vector<bool>& letterClicked, int index, int& clickedLetters, int& correctAnswers) {
		if (button == sf::Mouse::Left) {
			if (isValidLetter(letter)) {
				(*sounds[1]).play();
				clickedLetters++;
				letterClicked[index] = true;
				correctAnswers++;
			}
			else {
				(*sounds[2]).play();
				mistakes++;
			}
		}
		else if (button == sf::Mouse::Right) {
			if (!isValidLetter(letter)) {
				(*sounds[1]).play();
				clickedLetters++;
				letterClicked[index] = true;
				correctAnswers++;
			}
			else {
				(*sounds[2]).play();
				mistakes++;
			}
		}
	}

	void drawLetter(const Letter& letter) {
		if (letter.stampType == StampType::Circle || letter.stampType == StampType::Rectangle) {
			letter.shape->setPosition(letter.position);
			window.draw(*letter.shape);
		}
		else if (letter.stampType == StampType::Sprite) {
			letter.sprite->setPosition(letter.position);
			window.draw(*letter.sprite);
		}

		sf::Text cityRef("City: " + letter.city, font, 25);
		cityRef.setPosition(letter.position.x + 100, letter.position.y);
		window.draw(cityRef);
	}

	void drawScore() {
		sf::Text correctAnswersText("Correct: " + std::to_string(score), font, 25);
		correctAnswersText.setPosition(window.getSize().x - 200, window.getSize().y - 50);
		window.draw(correctAnswersText);

		sf::Text wrongAnswersText("Wrong: " + std::to_string(mistakes), font, 25);
		wrongAnswersText.setPosition(window.getSize().x - 200, window.getSize().y - 25);
		window.draw(wrongAnswersText);
	}

	void muteMusic() {
		for (int i = 0; i < sounds.size(); i++) {
			(*sounds[i]).setVolume(0);
			(*sounds[i]).stop();
		}
	}

	void exit() {
		muteMusic();
		window.close();
	}

	sf::RenderWindow window;
	std::vector<Letter> letters;
	std::vector<Letter> references;
	std::vector<std::string> textureFiles = { "sprite - 1.jpg", "sprite - 2.jpg" };
	std::vector<std::string> soundFiles = { "click.wav", "correct.wav", "mistake.wav", "music_death.mp3", "music_game.mp3", "music_victory.mp3" };
	std::vector<std::string> cities = {
		"Saint-Artztotzkagrad",
		"Artztotzkagrad",
		"Kolechiagrad",
		"Saintgrad",
		"Petrograd",
		"Saint-Petrograd",
		"Kolechiaburg",
		"Saint-Kolechiagrad",
		"Antegria City",
		"Antegriaburg",
		"New Grestin",
		"Saint-Grestin",
		"Grestin",
		"Orvech-on-the-Ob",
		"Orwell-on-the-Ob",
		"Impor City",
		"Imposterburg",
		"East Grestin",
		"West Grestin",
		"Kolechia Park",
		"Kolechia Hills",
		"Korista Valley",
		"Nirsk Falls",
		"Nirvana Falls",
		"Alexoville",
		"Alohacity",
		"Vostok Hills",
		"Vostok",
	};
	std::vector<sf::Color> colors = {
		sf::Color(244, 140, 186, 255),  // Пастельный розовый
		sf::Color(149, 205, 209, 255),  // Пастельный бирюзовый
		sf::Color(255, 224, 130, 255),  // Пастельный желтый
		sf::Color(189, 139, 196, 255),  // Пастельный лавандовый
		sf::Color(169, 204, 151, 255),  // Пастельный зеленый
		sf::Color(255, 163, 132, 255),  // Пастельный персиковый
		sf::Color(132, 158, 186, 255),  // Пастельный серо-синий
		sf::Color(229, 187, 205, 255)   // Пастельный розово-лиловый
	};
	std::vector<sf::Texture> textures;
	std::vector<std::unique_ptr<sf::Music>> sounds;
	sf::Texture back;
	sf::Sprite backgroundSprite;
	std::random_device rd;
	sf::Font font;
	int score = 0;
	int mistakes = 0;
	int day = 1;
};

int main() {
	Game game;
	game.run();
	return 0;
}