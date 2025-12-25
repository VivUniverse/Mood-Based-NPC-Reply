#include <iostream>
#include <SDL3/SDL.h>
#include <vector>
#include <map>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <SDL3_ttf/SDL_ttf.h>


std::string toLower(std::string& text) {
	std::transform(text.begin(), text.end(), text.begin(),
		[](unsigned char c) {return tolower(c); });
	return text;
}

class Chat {
public:
	std::string inputText = "";
	std::vector<std::string> history;
	SDL_FRect textureRect;
	SDL_FRect npcTextRect;

	// Moods
	std::map<std::string, std::string> ANGRY;
	std::map<std::string, std::string> HAPPY;
	std::map<std::string, std::string> NEUTRAL;

	enum Mood { angry, happy, neutral };
	Mood mood = neutral;

	int helloCount = 0;
	int byeCount = 0;
	int sorryCount = 0;

	// Methods
	void textEvent(const SDL_Event& event, bool chatOn) {

		if (chatOn) {
			if (event.type == SDL_EVENT_TEXT_INPUT) {
				inputText += event.text.text;
			}

			toLower(inputText);

			if (event.type == SDL_EVENT_KEY_DOWN) {
				SDL_Keycode key = event.key.key;
				switch (key) {
				case SDLK_BACKSPACE:
					if (!inputText.empty()) {
						inputText.pop_back();
					}
					break;

				case SDLK_RETURN:
					std::cout << "YOU: "
						<< inputText << std::endl;

					if (inputText == "hello") helloCount++;
					if (inputText == "bye")   byeCount++;
					if (inputText == "sorry") sorryCount++;

					history.push_back(inputText);
					inputText.clear(); break;
				}
			}
		}
	}

	void displayTextOnWindow(SDL_Renderer* renderer,
		TTF_Font* font, float scale) {
		SDL_Color color = { 0,0,0,255 };
		SDL_Surface* textSurface = TTF_RenderText_Blended(
			font, inputText.c_str(), 0, color
		);

		if (textSurface) {
			SDL_Texture* textTexture = SDL_CreateTextureFromSurface(
				renderer, textSurface
			);

			float tw, th;
			SDL_GetTextureSize(textTexture, &tw, &th);

			textureRect.w = tw * scale;
			textureRect.h = th * scale;

			SDL_RenderTexture(renderer, textTexture,
				nullptr, &textureRect
			);

			SDL_DestroyTexture(textTexture);
		}
		SDL_DestroySurface(textSurface);
	}

	void updateMood() {
		if (helloCount > byeCount && helloCount > sorryCount) { mood = happy; }
		else if (byeCount > helloCount && byeCount > sorryCount) { mood = angry; }
		else if (sorryCount > helloCount && sorryCount > byeCount) { mood = neutral; }
	}

	std::string getNpcReply(std::string& keyword, Chat::Mood mood) {
		if (mood == angry) {
			if (keyword == "hello") return "No more talks!";
			if (keyword == "bye")   return "Get out of my SIGHT fool!";
			if (keyword == "sorry") return "No more chats...";
		}
		else if (mood == happy) {
			if (keyword == "hello") return "Hello my friend";
			if (keyword == "bye")   return "Farewell friend...";
			if (keyword == "sorry") return "No more apologies!";
		}
		else if (mood == neutral) {
			if (keyword == "hello") return "Hello traveller";
			if (keyword == "bye")   return "Goodbye traveller...";
			if (keyword == "sorry") return "It's alright!";
		}

		return "..."; // fallback

	}

	void npcMoodReply(const SDL_Event& event) {
		if (event.type == SDL_EVENT_KEY_DOWN &&
			event.key.key == SDLK_RETURN) {

			if (!history.empty()) {
				std::string last = history.back();
				std::string reply = getNpcReply(last, mood);

				std::cout << "Mage: "
					<< reply << std::endl;
			}
		}
	}

	void displayNpcTextOnWindow(SDL_Renderer* renderer, TTF_Font* font,
		const SDL_Event& event, float scale) {
		SDL_Color color = { 0,0,0,255 };

		if (!history.empty()) {
			std::string last = history.back();

			SDL_Surface* npcTextSurface = TTF_RenderText_Blended(
				font, getNpcReply(last, mood).c_str(),
				0, color
			);

			if (npcTextSurface) {
				SDL_Texture* npcTextTexture = SDL_CreateTextureFromSurface(
					renderer, npcTextSurface
				);

				float tw, th;
				SDL_GetTextureSize(npcTextTexture, &tw, &th);

				npcTextRect.w = tw * scale;
				npcTextRect.h = th * scale;

				SDL_RenderTexture(renderer, npcTextTexture,
					nullptr, &npcTextRect);

				SDL_DestroyTexture(npcTextTexture);
			}
			SDL_DestroySurface(npcTextSurface);
		}
	}
};


class Entity {
public:
	SDL_Texture* texture;
	SDL_FRect rect;
	float gravity = 0.5f;
	float x_vel = 5.0f;
	float y_vel = 0.0f;

};


class PhysicsBody : public Entity {
public:
	// Constructor
	PhysicsBody(float x, float y, float scale) {
		rect.x = x;
		rect.y = y;
	}

	void applyGravity(float floorY) {
		y_vel += gravity;
		rect.y += y_vel;

		if (rect.y + rect.h >= floorY) {
			rect.y = floorY - rect.h;
			y_vel = 0;
		}
	}

	void jump(float power) {
		y_vel = -power;
	}
};


class Hero : public PhysicsBody {
private:
	bool aDown = false, dDown = false;
public:
	// Constructor
	Hero(SDL_Renderer* renderer, float scale) :
		PhysicsBody(100, 100, scale) {
		texture = IMG_LoadTexture(
			renderer, "Npc/Hero.png"
		);

		float tw, th;
		SDL_GetTextureSize(texture, &tw, &th);

		rect.w = tw * scale;
		rect.h = th * scale;
	}

	void keyEvent(const SDL_Event& event, bool chatOn,
		float floorY) {
		SDL_Keycode key = event.key.key;
		bool onGround = rect.y + rect.h >= floorY;

		if (!chatOn) {
			if (event.type == SDL_EVENT_KEY_DOWN) {
				switch (key) {
				case SDLK_A: aDown = true; break;
				case SDLK_D: dDown = true; break;
				case SDLK_SPACE:
					if (onGround) {
						jump(12);
					}
					break;
				}
			}

			if (event.type == SDL_EVENT_KEY_UP) {
				switch (key) {
				case SDLK_A: aDown = false; break;
				case SDLK_D: dDown = false; break;
				}
			}
		}
		// Key controls
		if (aDown) { rect.x -= x_vel; }
		if (dDown) { rect.x += x_vel; }
	}

	void draw(SDL_Renderer* renderer) {
		SDL_RenderTexture(renderer, texture,
			nullptr, &rect);
	}
};

class Mage : public PhysicsBody {
public:
	// Constructor
	Mage(SDL_Renderer* renderer, float scale) :
		PhysicsBody(600, 100, scale) {
		texture = IMG_LoadTexture(
			renderer, "Npc/Mage.png"
		);

		float tw, th;
		SDL_GetTextureSize(texture, &tw, &th);

		rect.w = tw * scale;
		rect.h = th * scale;
	}

	void draw(SDL_Renderer* renderer) {
		SDL_RenderTexture(renderer, texture,
			nullptr, &rect);
	}
};




int main(int argc, char* argv[]) {
	// Initilize
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init Error: "
			<< SDL_GetError() << std::endl;
		return 1;
	}

	if (!TTF_Init()) {
		std::cerr << "TTF_Init Error: "
			<< SDL_GetError() << std::endl;
		return 1;
	}

	// Game Varibles
	int WIDTH = 1000;
	int HEIGHT = 800;

	// Create Rects
	SDL_FRect floor;
	floor.x = 0;
	floor.y = 700;
	floor.w = WIDTH + 50;
	floor.h = 100;

	// Load Fonts
	TTF_Font* font = TTF_OpenFont(
		"Fonts/OpenSans-Italic.ttf", 24
	);

	// Create Window And Renderer
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	SDL_CreateWindowAndRenderer(
		"SDL3 Window Created",
		WIDTH, HEIGHT,
		SDL_WINDOW_RESIZABLE,
		&window, &renderer
	);

	if (!window || !renderer) {
		std::cerr
			<< "SDL_CreateWindowAndRenderer Error: "
			<< SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	// Create Objects
	Chat chat;
	Hero hero(renderer, 0.1);
	Mage mage(renderer, 0.15);

	// Main Loop
	bool isRunning = true;
	SDL_Event event;
	bool chatOn = false;
	SDL_StartTextInput(window);

	while (isRunning) {
		// Event Loop
		while (SDL_PollEvent(&event)) {
			SDL_Keycode key = event.key.key;

			if (event.type == SDL_EVENT_QUIT) {
				isRunning = false;
			}

			if (event.type == SDL_EVENT_KEY_DOWN) {
				switch (key) {
				case SDLK_LSHIFT: chatOn = true; break;
				case SDLK_RSHIFT: chatOn = false; break;
				}
			}

			chat.textEvent(event, chatOn);
			hero.keyEvent(event, chatOn, floor.y);
			chat.updateMood();
			chat.npcMoodReply(event);

			if (!chatOn) {
				if (event.type == SDL_EVENT_KEY_DOWN) {
					SDL_Keycode key = event.key.key;
					switch (key) {
					case SDLK_H:
						std::cout << "---History---\n";
						for (int i = 0; i < chat.history.size(); i++) {
							std::cout << "- " << chat.history[i] << std::endl;
						}
						break;
					}
				}
			}
		}
		// Fill Screen
		SDL_SetRenderDrawColor(
			renderer, 0, 248, 255, 255
		);
		SDL_RenderClear(renderer);

		// Graphics
		hero.draw(renderer);
		mage.draw(renderer);
		chat.displayTextOnWindow(renderer, font, 0.8);
		chat.displayNpcTextOnWindow(renderer, font, event, 0.8);

		SDL_SetRenderDrawColor(
			renderer, 150, 75, 0, 255
		);
		SDL_RenderFillRect(renderer, &floor);

		// Apply Graphics
		hero.applyGravity(floor.y);
		mage.applyGravity(floor.y);

		chat.textureRect.x = hero.rect.x + (hero.rect.w / 2) - (chat.textureRect.w / 2);
		chat.textureRect.y = hero.rect.y - chat.textureRect.h - 10;

		chat.npcTextRect.x = mage.rect.x + 100 + (mage.rect.w / 2) - (mage.rect.w / 2);
		chat.npcTextRect.y = mage.rect.y - chat.npcTextRect.h - 10;

		// Present
		SDL_RenderPresent(renderer);

		// FPS
		SDL_Delay(16);
	}
	// Clean Up
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 0;
}