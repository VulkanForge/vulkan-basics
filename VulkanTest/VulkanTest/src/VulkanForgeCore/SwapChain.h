#ifndef VULKAN_FORGE_SWAP_CHAIN
#define VULKAN_FORGE_SWAP_CHAIN

#include <SDL.h>
#include <vulkan.h>
#include "Result.h"


class Device;
class SwapChain {

public:
    Device* owningDevice;

    VkSwapchainKHR vkSwapChain;
    
    Result Init();

};


#endif