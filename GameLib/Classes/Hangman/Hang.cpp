#include "Hang.h"

Hang::Hang(SDL_Renderer* ren, SDL_Window* win, std::string username) {
	//	Initialize SDL
	this->ren = ren;
	this->win = win;
	this->username = username;
	SDL_GetWindowSize(this->win, &winWidth, &winHeight);

	//	Initialize TTF
	TTF_Init();
	SDL_StartTextInput();

	//	Initialize audio
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		std::cout << "Failed at Mi_OpenAudio()" << std::endl;
	}
	Mix_Volume(-1, 100);
	Mix_VolumeMusic(5);
	bgm = Mix_LoadMUS("Resources/Audio/HangBgm.wav");
	clickEffect = Mix_LoadWAV("Resources/Audio/Click.wav");
	thankEffect = Mix_LoadWAV("Resources/Audio/Thanks.wav");
	loseEffect = Mix_LoadWAV("Resources/Audio/Lose.wav");
	winnerEffect = Mix_LoadWAV("Resources/Audio/Win.wav");
	screamEffect = Mix_LoadWAV("Resources/Audio/Scream3.wav");
	errorEffect = Mix_LoadWAV("Resources/Audio/Error.wav");
	volume.setDest(winWidth - 25, 0, 25, 25);
	volume.setImage("Resources/Assets/GameImage/Volume.png", ren);
	volume.setSource(0, 0, 50, 50);

	//	Initialize title and fonts
	head = "HANGMAN";
	titleFont = TTF_OpenFont("Resources/Fonts/Chopsic/Chopsic-K6Dp.ttf", 48);
	normalFont = TTF_OpenFont("Resources/Fonts/Montserrat/MontserratLight-ywBvq.ttf", 24);
	playerFont = TTF_OpenFont("Resources/Fonts/Montserrat/MontserratLight-ywBvq.ttf", 56);
	instructionFont = TTF_OpenFont("Resources/Fonts/Montserrat/MontserratLight-ywBvq.ttf", 16);
	
	//	Open file containing words and select a random one
	srand(time(NULL));
	file.open("Resources/Assets/Hangman/hangwords.txt");
	while (file >> word) {
		std::transform(word.begin(), word.end(), word.begin(), ::toupper);
		if (word.length() <= 10) {
			words.push_back(word);
		}
	}
	word = words[(rand() % 61333)];
	int nTmpGuess = rand() % word.length();
	guess += (word[nTmpGuess]);

	//	Chances
	chances = 5;

	//	Initialize remaining variables
	FPS = 60;
	frameDelay = 1000 / FPS;
	anim.setDest(1, winHeight - 550, 500, 500);
	anim.setImage("Resources/Assets/Hangman/Anim.png", ren);
	anim.setSource(0, 0, 500, 500);
	winner = -1;
	isMute = false;
	game = 1;
	running = 1;
}

//	Hang Destructor
Hang::~Hang() {
	file.close();
	Mix_FreeChunk(clickEffect);
	Mix_FreeChunk(loseEffect);
	Mix_FreeChunk(winnerEffect);
	Mix_FreeChunk(screamEffect);
	Mix_FreeChunk(thankEffect);
	Mix_FreeMusic(bgm);
	Mix_Quit();
	TTF_CloseFont(titleFont);
	TTF_CloseFont(normalFont);
	TTF_CloseFont(playerFont);
	SDL_StopTextInput();
	TTF_Quit();
}

//	Hang main loop
void Hang::loop() {
	Mix_PlayMusic(bgm, -1);
	while (running) {
		frameStart = SDL_GetTicks();

		render();
		input();

		frameTime = SDL_GetTicks() - frameStart;

		if (frameDelay > frameTime) {
			SDL_Delay(frameDelay - frameTime);
		}
	}
	Mix_HaltMusic();
	if (game) {
		game = 1;
		if (winner == 0) {
			game = 3;
			Mix_PlayChannel(-1, loseEffect, 0);
		}
		else {
			Mix_PlayChannel(-1, winnerEffect, 0);
		}
		FPS = 1;
		while (game) {
			frameStart = SDL_GetTicks();

			render();
			updateAnimation();

			frameTime = SDL_GetTicks() - frameStart;

			if (frameDelay > frameTime) {
				SDL_Delay(frameDelay - frameTime);
			}
		}
		SDL_Delay(4000);
	}
}

//	Hang update animation
void Hang::updateAnimation() {
	game--;
	SDL_Rect tmpRect;
	tmpRect = anim.getSource();
	if (winner) {
		if (game == 0) {
			Mix_PlayChannel(-1, thankEffect, 0);
		}
		anim.setSource(tmpRect.x + 500, tmpRect.y + 500, 500, 500);
	}
	else {
		Mix_PlayChannel(-1, screamEffect, 0);
		anim.setSource(tmpRect.x + 500, tmpRect.y, 500, 500);
	}
}

//	Hang render
void Hang::render() {
	SDL_RenderClear(ren);

	int textWidth, textHeight;

	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_Rect rect;
	rect.w = winWidth;
	rect.h = winHeight;
	rect.x = 0;
	rect.y = 0;
	SDL_RenderFillRect(ren, &rect);

	//	Head Text
	TTF_SizeText(titleFont, head, &textWidth, &textHeight);
	draw(head, titleFont, (winWidth / 2) - (textWidth / 2), 10, 255, 255, 0);

	SDL_SetRenderDrawColor(ren, 135, 206, 235, 255);
	SDL_RenderDrawLine(ren, 1, winHeight - 50, 501, winHeight - 50);
	SDL_RenderDrawLine(ren, 1, winHeight - 600, 501, winHeight - 600);
	SDL_RenderDrawLine(ren, 1, winHeight - 50, 1, winHeight - 600);
	SDL_RenderDrawLine(ren, 501, winHeight - 50, 501, winHeight - 600);
	
	SDL_Rect animRect;
	animRect.w = 500;
	animRect.h = 550;
	animRect.x = 1;
	animRect.y = winHeight - 600;
	SDL_RenderFillRect(ren, &animRect);

	draw();

	draw(anim);

	SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);

	//	Username
	int tmpW, tmpH;
	TTF_SizeText(instructionFont, "User : ", &tmpW, &tmpH);
	draw("User : ", instructionFont, 0, winHeight - tmpH, 255, 255, 0);
	draw(username.c_str(), instructionFont, tmpW, winHeight - tmpH, 0, 255, 0);

	SDL_RenderPresent(ren);
}

//	Hang draw
void Hang::draw() {
	int textHeight, textWidth;
	int spaceWidth, spaceHeight;
	TTF_SizeText(normalFont, "A", &spaceWidth, &spaceHeight);
	int totalWidth = 26 * spaceWidth;
	std::string s;
	int i = 0;
	int j = 250;

	//	Set render draw color
	SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);

	//	All the letters
	SDL_Rect availRect;
	availRect.x = 501 + (winWidth - 501 - totalWidth) / 2 - spaceWidth;
	availRect.y = j - 5;
	availRect.w = totalWidth + 10 + spaceWidth;
	availRect.h = spaceWidth * 2 + 10 + 50;
	SDL_RenderDrawRect(ren, &availRect);
	for (char ch = 'A'; ch <= 'Z'; ch++) {
		s = ch;
		if (guess.find(ch) != std::string::npos && word.find(ch) == std::string::npos) {
			draw(s.c_str(), normalFont, 501 + (winWidth - 501 - totalWidth) / 2 + i * 2 * spaceWidth, j, 255, 0, 0);
		}
		else if (guess.find(ch) != std::string::npos && word.find(ch) != std::string::npos) {
			draw(s.c_str(), normalFont, 501 + (winWidth - 501 - totalWidth) / 2 + i * 2 * spaceWidth, j, 0, 255, 0);
		}
		else {
			draw(s.c_str(), normalFont, 501 + (winWidth - 501 - totalWidth) / 2 + i * 2 * spaceWidth, j, 255, 255, 255);
		}
		i++;
		if (ch == 'M') {
			i = 0;
			j += 50;
		}
	}

	//	The random word
	totalWidth = word.length() * 2 * spaceWidth;
	SDL_Rect randRect;
	randRect.x = 496 + (winWidth - 501 - totalWidth) / 2 - spaceWidth;
	randRect.y = 395;
	randRect.w = totalWidth + 10 + spaceWidth;
	randRect.h = spaceHeight + 10;
	SDL_RenderDrawRect(ren, &randRect);
	if (winner == 0) {
		for (i = 0; i < word.length(); i++) {
			s = word[i];
			int r = 255, g = 255, b = 255;
			if (guess.find(word[i]) == std::string::npos) {
				r = 0;
				b = 0;
			}
			draw(s.c_str(), normalFont, 501 + (winWidth - 501 - totalWidth) / 2 + i * 2 * spaceWidth, 400, r, g, b);
		}
	}
	else {
		for (i = 0; i < word.length(); i++) {
			if (guess.find(word[i]) != std::string::npos) {
				s = word[i];
			}
			else {
				s = "_";
			}
			draw(s.c_str(), normalFont, 501 + (winWidth - 501 - totalWidth) / 2 + i * 2 * spaceWidth, 400, 255, 255, 255);
		}
	}

	//	On getting a winner or not
	SDL_Rect winRect;
	if (winner == 0) {
		TTF_SizeText(normalFont, "You lost !!!", &textWidth, &textHeight);
		winRect.x = 496 + (winWidth - 501 - textWidth) / 2;
		winRect.y = 495;
		winRect.w = textWidth + 10;
		winRect.h = textHeight + 10;
		SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
		SDL_RenderDrawRect(ren, &winRect);
		draw("You lost !!!", normalFont, 501 + (winWidth - 501 - textWidth) / 2, 500, 255, 0, 0);
	}
	else if (winner == 1) {
		TTF_SizeText(normalFont, "You won !!!", &textWidth, &textHeight);
		winRect.x = 496 + (winWidth - 501 - textWidth) / 2;
		winRect.y = 495;
		winRect.w = textWidth + 10;
		winRect.h = textHeight + 10; 
		SDL_SetRenderDrawColor(ren, 0, 255, 0, 255);
		SDL_RenderDrawRect(ren, &winRect);
		draw("You won !!!", normalFont, 501 + (winWidth - 501 - textWidth) / 2, 500, 0, 255, 0);
	}
	else {
		std::string moves = "Wrong Moves Left : " + std::to_string(chances);
		TTF_SizeText(normalFont, moves.c_str(), &textWidth, &textHeight);
		draw(moves.c_str(), normalFont, 501 + (winWidth - 501 - textWidth) / 2, 150, 0, 0, 255);
	}
	
	//	Instructions
	TTF_SizeText(instructionFont, "Press ESC to close", &textWidth, &textHeight);
	draw("Press ESC to close", instructionFont, winWidth - textWidth, winHeight - textHeight, 191, 191, 63);

	//	Mute button
	draw(volume);
}

//	Hang draw object
void Hang::draw(Object obj) {
	SDL_Rect dest = obj.getDest();
	SDL_Rect src = obj.getSource();
	SDL_RenderCopyEx(ren, obj.getTex(), &src, &dest, 0, NULL, SDL_FLIP_NONE);
}

//	Hang draw text
void Hang::draw(const char* msg, TTF_Font* font, int x, int y, int r, int g, int b) {
	SDL_Surface* surfMsg;
	SDL_Texture* texMsg;
	SDL_Rect rect;
	SDL_Color color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = 255;
	surfMsg = TTF_RenderText_Solid(font, msg, color);
	texMsg = SDL_CreateTextureFromSurface(ren, surfMsg);
	rect.x = x;
	rect.y = y;
	rect.w = surfMsg->w;
	rect.h = surfMsg->h;
	SDL_RenderCopy(ren, texMsg, NULL, &rect);
	SDL_FreeSurface(surfMsg);
	SDL_DestroyTexture(texMsg);
}

//	Hang get winner
int Hang::getWinner() {
	for (int i = 0; i < word.length(); i++) {
		if (guess.find(word[i]) == std::string::npos) {
			if (chances < 0) {
				anim.setSource(500, 0, 500, 500);
				return 0;
				chances++;
			}
			else {
				return -1;
			}
		}
	}
	anim.setSource(500, 500, 500, 500);
	return 1;
}

//	Hang input
void Hang::input() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {

		//	Quit button
		if (event.type == SDL_QUIT) {
			running = 0;
			std::cout << "Quitting" << std::endl;
		}

		//	On key press
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				running = 0;
				game = 0;
			}
		}

		//	On mouse click
		if (event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.motion.x > winWidth - 30 && event.motion.x < winWidth) {
				if (event.motion.y > 0 && event.motion.y < 30) {
					if (isMute) {
						Mix_Volume(-1, 100);
						Mix_VolumeMusic(5);
						volume.setSource(0, 0, 50, 50);
					}
					else {
						Mix_Volume(-1, 0);
						Mix_VolumeMusic(0);
						volume.setSource(50, 0, 50, 50);
					}
					isMute = !isMute;
				}
			}
		}

		if (event.type == SDL_TEXTINPUT && chances > -1) {
			std::string ch = event.text.text;
			if (ch.length() == 1) {
				std::transform(ch.begin(), ch.end(), ch.begin(), ::toupper);
				if ((guess.find(ch)) == std::string::npos) {
					Mix_PlayChannel(-1, clickEffect, 0);
					guess += ch;

					if (word.find(ch) == std::string::npos) {
						Mix_PlayChannel(-1, errorEffect, 0);
						chances--;
					}

					winner = getWinner();
					if (winner != -1) {
						running = 0;
					}
				}
			}
		}
	}
}