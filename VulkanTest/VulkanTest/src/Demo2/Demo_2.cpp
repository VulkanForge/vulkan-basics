#include "Demo_2.h"
#include <iostream>

Demo_2::Demo_2() : running(false), mainwindow(NULL) {}

void Demo_2::OnEvent(SDL_Event* event) {
	switch (event->type) {
	case SDL_MOUSEBUTTONUP:
		std::cout << "MOUSE_BUTTON_UP: x:" << event->button.x << "| y:"
			<< event->button.y
			<< "| button1:" << (event->button.button == SDL_BUTTON_LEFT)
			<< "| button2:" << (event->button.button == SDL_BUTTON_RIGHT)
			<< "| button3:" << (event->button.button == SDL_BUTTON_MIDDLE)
			<< "| clickState:" << event->motion.state
			<< std::endl << std::endl;
		break;
	case SDL_MOUSEBUTTONDOWN:
		std::cout << "MOUSE_BUTTON_DOWN: x:" << event->button.x
			<< "| y:" << event->button.y
			<< "| button1:" << (event->button.button == SDL_BUTTON_LEFT)
			<< "| button2:" << (event->button.button == SDL_BUTTON_RIGHT)
			<< "| button3:" << (event->button.button == SDL_BUTTON_MIDDLE)
			<< "| clickState:" << event->motion.state
			<< std::endl << std::endl;
		break;
	case SDL_MOUSEMOTION:
		std::cout << "MOUSE_MOTION: x:" << event->motion.x
			<< "| y:" << event->motion.y
			<< "| xrel:" << event->motion.xrel
			<< "| yrel:" << event->motion.yrel
			<< "| clickState:" << event->motion.state
			<< "| button1:" << ((event->motion.state & 1) == 1)
			<< "| button2:" << ((event->motion.state & 4) == 4)
			<< "| button3:" << ((event->motion.state & 2) == 2)
			<< std::endl << std::endl;
		break;
	case SDL_KEYUP:
		running = event->key.keysym.sym != SDLK_ESCAPE;
		break;
	case SDL_QUIT:
		running = false;
		break;
	default:
		break;
	}
}
void Demo_2::OnLoop() {}
void Demo_2::OnRender() {}

void Demo_2::OnCleanup() {
	SDL_DestroyWindow(mainwindow);
	SDL_Quit();
}

bool Demo_2::OnInit() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		return false;

	mainwindow = SDL_CreateWindow("Titulo de la Ventana",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH, WIN_HEIGHT,
		SDL_WINDOW_SHOWN);
	if (!mainwindow)
		return false;

	running = true;

	return true;
}

int Demo_2::OnExecute() {
	if (!Init())
		return -1;

	SDL_Event event;

	while (running) {

		while (SDL_PollEvent(&event))
			Event(&event);

		Loop();
		Render();
	}

	Cleanup();

	return 0;
}