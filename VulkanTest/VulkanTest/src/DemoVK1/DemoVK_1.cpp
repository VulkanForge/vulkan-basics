#include "DemoVK_1.h"
#include <iostream>

DemoVK_1::DemoVK_1() : _running(false), _mainwindow(NULL) {}

void DemoVK_1::OnEvent(SDL_Event* event) {
	switch (event->type) {
	case SDL_KEYUP:
		switch (event->key.keysym.sym) {
		case SDLK_ESCAPE:
			_running = false;
			break;
		default:
			break;
		}
		break;
	case SDL_QUIT:
		_running = false;
		break;
	default:
		break;
	}
}
void DemoVK_1::OnLoop() {}
void DemoVK_1::OnRender() {
	SDL_GL_SwapWindow(_mainwindow);
}

void DemoVK_1::OnCleanup() {
	VulkanCommon::DestroyInstance(_vulkanInfo);

	SDL_GL_DeleteContext(_ctxt);
	SDL_DestroyWindow(_mainwindow);
	SDL_Quit();
}

bool DemoVK_1::OnInit() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "cannot initialise SDL environment\n";
		return false;
	}

	_mainwindow = SDL_CreateWindow("Vulkan Forge",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH, WIN_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!_mainwindow) {
		std::cerr << "cannot create a SDL window\n";
		return false;
	}

	SetupContext();

	if (!InitVulkan()) return false;

	_running = true;
	
	return true;
}

int DemoVK_1::OnExecute() {
	if (!Init())
		return -1;

	SDL_Event event;

	while (_running) {

		while (SDL_PollEvent(&event))
			Event(&event);

		Loop();
		Render();
	}

	Cleanup();

	return 0;
}

void DemoVK_1::SetupContext() {
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	_ctxt = SDL_GL_CreateContext(_mainwindow);

	//vsync ON
	SDL_GL_SetSwapInterval(1);
}

bool DemoVK_1::InitVulkan() {
	// Create Vulkan Instance
	VkResult vkResult = VulkanCommon::CreateInstance(_vulkanInfo);

	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER) {
		std::cerr << "cannot find a compatible Vulkan ICD\n";
		return false;
	}
	else if (vkResult) {
		std::cerr << "unknown error\n";
		return false;
	}

	// Enumerate Physical Devices
	vkResult = VulkanCommon::CheckDevices(_vulkanInfo);

	if (vkResult != VK_SUCCESS || _vulkanInfo.gpus.size() == 0) {
		std::cerr << "error enumerating physical devices. gpus: " << _vulkanInfo.gpus.size() <<
			" - VkResult: " << vkResult << "\n";
		return false;
	}
	return true;
}