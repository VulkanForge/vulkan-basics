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
	VulkanCommon::FreeCommandBuffer(_vulkanInfo);
	VulkanCommon::DestroyCommandPool(_vulkanInfo);
	VulkanCommon::DestroyDevice(_vulkanInfo);
	VulkanCommon::DestroyInstance(_vulkanInfo);

	SDL_GL_DeleteContext(_ctxt);
	SDL_DestroyWindow(_mainwindow);
	SDL_Quit();
}

bool DemoVK_1::OnInit() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "cannot initialise SDL environment" << std::endl;
		return false;
	}

	_mainwindow = SDL_CreateWindow("Vulkan Forge",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH, WIN_HEIGHT,
		SDL_WINDOW_SHOWN/* | SDL_WINDOW_OPENGL*/);
	if (!_mainwindow) {
		std::cerr << "cannot create a SDL window" << std::endl;
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

	_vulkanInfo.width = WIN_WIDTH;
	_vulkanInfo.height = WIN_HEIGHT;

	VulkanCommon::VulkanForge_outcome outcome = { VkResult::VK_SUCCESS, VulkanCommon::VulkanForge_Result::SUCCESS };

	// Create Vulkan Instance
	VkResult vkResult = VulkanCommon::CreateInstance(_vulkanInfo);

	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER) {
		std::cerr << "cannot find a compatible Vulkan ICD" << std::endl;
		return false;
	}
	else if (vkResult) {
		std::cerr << "unknown error" << std::endl;
		return false;
	}
	std::cout << "Vulkan Instance Created" << std::endl;


	// Enumerate Physical Devices
	vkResult = VulkanCommon::CheckDevices(_vulkanInfo);

	if (vkResult != VK_SUCCESS || _vulkanInfo.gpus.size() == 0) {
		std::cerr << "error enumerating physical devices. gpus: " << _vulkanInfo.gpus.size() <<
			" - VkResult: " << vkResult << std::endl;
		return false;
	}
	std::cout << "Number of devices detected: " << _vulkanInfo.gpus.size() << std::endl;


	//Create Vulkan Device
	bool found = VulkanCommon::TryGetGraphicQueue(_vulkanInfo);
	if (!found) {
		std::cerr << "error finding graphic Queue" << std::endl;
		return false;
	}

	vkResult = VulkanCommon::CreateDevice(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.device == NULL) {
		std::cerr << "error creating vulkan device - VkResult: " << vkResult << std::endl;
		return false;
	}

	std::cout << "Vulkan device created" << std::endl;

	// Attach SDL window
	SDL_SysWMinfo sdlSysInfo;
	SDL_VERSION(&sdlSysInfo.version);
	SDL_GetWindowWMInfo(_mainwindow, &sdlSysInfo);
	//TCHAR* className;
	char className[256];
	GetClassName(sdlSysInfo.info.win.window, className, 256);
	WNDCLASS wce;
	GetClassInfo(GetModuleHandle(NULL), className, &wce);
	//VkWin32SurfaceCreateInfoKHR createInfo;
	_vulkanInfo.connection = wce.hInstance;
	_vulkanInfo.window = sdlSysInfo.info.win.window;

	//Create Surface Descriptor
	vkResult = VulkanCommon::CreateSurfaceDescription(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.surface == NULL) {
		std::cerr << "Could not create a Surface Descriptor - VkResult: " << vkResult << std::endl;
		return false;
	}

	std::cout << "Surface descriptor created" << std::endl;


	//Try Get Graphic And Present Queue
	if (!VulkanCommon::TryGetGraphicAndPresentQueue(_vulkanInfo)) {
		std::cerr << "Could not find a queue that supports both graphics and present" << std::endl;
		return false;
	}
	
	std::cout << "Found Graphics And Present Queue" << std::endl;

	//Initialize Device Surface and Swap Chain
	vkResult = VulkanCommon::InitDeviceSurfaceAndSwapChain(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.format == VK_FORMAT_UNDEFINED) {
		std::cerr << "error initializing device surface or swap chain - VkResult: " << vkResult << std::endl;
		return false;
	}

	std::cout << "Device Surface and Swap Chain initialized" << std::endl;


	//Create Command Pool
	vkResult = VulkanCommon::CreateCommandPool(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.commandPool == NULL) {
		std::cerr << "error creating command Pool - VkResult: " << vkResult << std::endl;
		return false;
	}

	std::cout << "Command Pool created" << std::endl;

	//Create Command Buffer
	vkResult = VulkanCommon::CreateCommandBuffer(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.commandBuffer == NULL) {
		std::cerr << "error creating command Buffer - VkResult: " << vkResult << std::endl;
		return false;
	}

	std::cout << "Command Buffer created" << std::endl;

	//Begin Command Buffer
	vkResult = VulkanCommon::BeginCommandBuffer(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.commandBuffer == NULL) {
		std::cerr << "error beginning command Buffer - VkResult: " << vkResult << std::endl;
		return false;
	}
	std::cout << "Command Buffer Begin" << std::endl;

	//Populate Swap Chain Images
	vkResult = VulkanCommon::PopulateSwapChainImages(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.commandBuffer == NULL) {
		std::cerr << "error populating swap chain images - VkResult: " << vkResult << std::endl;
		return false;
	}
	std::cout << "Populated Swap Chain Images" << std::endl;


	//Create Depth Buuffer
	outcome = VulkanCommon::CreateDepthBuffer(_vulkanInfo);
	if (outcome.vkResult != VK_SUCCESS || outcome.vfResult != VulkanCommon::VulkanForge_Result::SUCCESS) {
		std::cerr << "error creating depth buffer - VkResult: " << outcome.vkResult << "  VfResult " << outcome.vfResult << std::endl;
		return false;
	}
	std::cout << "Created Depth Buffer" << std::endl;

	//Create Depth Buuffer
	outcome = VulkanCommon::CreateUniformBuffer(_vulkanInfo);
	if (outcome.vkResult != VK_SUCCESS || outcome.vfResult != VulkanCommon::VulkanForge_Result::SUCCESS) {
		std::cerr << "error creating uniform buffer - VkResult: " << outcome.vkResult << "  VfResult " << outcome.vfResult << std::endl;
		return false;
	}
	std::cout << "Created and Populated with MVP the Uniform Buffer" << std::endl;

	//Create Pipeline Layout
	outcome = VulkanCommon::CreatePipelineLayout(_vulkanInfo);
	if (outcome.vkResult != VK_SUCCESS || outcome.vfResult != VulkanCommon::VulkanForge_Result::SUCCESS) {
		std::cerr << "error creating pipeline layout - VkResult: " << outcome.vkResult << "  VfResult " << outcome.vfResult << std::endl;
		return false;
	}
	std::cout << "Created Pipeline Layout" << std::endl;

	//Init Render Pass
	outcome = VulkanCommon::InitRenderPass(_vulkanInfo);
	if (outcome.vkResult != VK_SUCCESS || outcome.vfResult != VulkanCommon::VulkanForge_Result::SUCCESS) {
		std::cerr << "error initializing render pass - VkResult: " << outcome.vkResult << "  VfResult " << outcome.vfResult << std::endl;
		return false;
	}
	std::cout << "Render pass initialized" << std::endl;

	//Init Shader
	outcome = VulkanCommon::InitShaders(_vulkanInfo);
	if (outcome.vkResult != VK_SUCCESS || outcome.vfResult != VulkanCommon::VulkanForge_Result::SUCCESS) {
		std::cerr << "error initializing shaders - VkResult: " << outcome.vkResult << "  VfResult " << outcome.vfResult << std::endl;
		return false;
	}
	std::cout << "Shaders initialized" << std::endl;

	//End Command Buffer
	vkResult = VulkanCommon::EndCommandBuffer(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.commandBuffer == NULL) {
		std::cerr << "error ending command buffer - VkResult: " << vkResult << std::endl;
		return false;
	}
	std::cout << "Ended Command Buffer" << std::endl;


	//Execute command buffer queue
	vkResult = VulkanCommon::ExecuteQueueCommandBuffer(_vulkanInfo);
	if (vkResult != VK_SUCCESS || _vulkanInfo.commandBuffer == NULL) {
		std::cerr << "error executing command buffer queue - VkResult: " << vkResult << std::endl;
		return false;
	}
	std::cout << "Executed Command Buffer Queue" << std::endl;

	return true;
}