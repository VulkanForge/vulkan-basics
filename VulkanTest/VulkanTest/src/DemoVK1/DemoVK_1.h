#ifndef DEMOVK_1_H_
#define DEMOVK_1_H_
#include "..\Common\Common.h"

class DemoVK_1 {
private:
	bool _running;
	SDL_Window* _mainwindow;
	SDL_GLContext _ctxt;
	VulkanCommon::VulkanForge_info _vulkanInfo;
	static const uint32_t WIN_HEIGHT = 512; //px
	static const uint32_t WIN_WIDTH = 512; //px
public:

	DemoVK_1();
	int Execute() { return OnExecute(); }
	bool Init() { return OnInit(); }
	void Loop() { return OnLoop(); }
	void Render() { return OnRender(); }
	void Cleanup() { return OnCleanup(); }
	void Event(SDL_Event* Event) { OnEvent(Event); }



	int OnExecute();

	bool OnInit();
	void OnEvent(SDL_Event* Event);
	void OnLoop();
	void OnRender();
	void OnCleanup();

	void SetupContext();
	bool InitVulkan();
};


#endif