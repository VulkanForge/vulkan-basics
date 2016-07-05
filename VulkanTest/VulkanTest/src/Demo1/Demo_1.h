#ifndef DEMO_1_H_
#define DEMO_1_H_
#include <SDL.h>

class Demo_1 {
private:
	SDL_Window* mainwindow;
	bool running;
	static const uint32_t WIN_HEIGHT = 512; //px
	static const uint32_t WIN_WIDTH = 512; //px

public:
	Demo_1();
	int Execute(){ return OnExecute(); }
	bool Init(){ return OnInit(); }
	void Loop(){ return OnLoop(); }
	void Render(){ return OnRender(); }
	void Cleanup(){ return OnCleanup(); }
	void Event(SDL_Event* Event){ OnEvent(Event); }


	int OnExecute();

	bool OnInit();
	void OnEvent(SDL_Event* Event);
	void OnLoop();
	void OnRender();
	void OnCleanup();

};


#endif