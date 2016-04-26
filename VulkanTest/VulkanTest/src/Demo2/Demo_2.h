#ifndef DEMO_2_H_
#define DEMO_2_H_
#include <SDL.h>

class Demo_2 {
private:
	bool running;
	SDL_Window* mainwindow;
	static const uint32_t WIN_HEIGHT = 512; //px
	static const uint32_t WIN_WIDTH = 512; //px
public:

	Demo_2();
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