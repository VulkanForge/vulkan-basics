#ifndef VULKAN_FORGE_INSTANCE_H_
#define VULKAN_FORGE_INSTANCE_H_


#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#else // _WIN32
#include <unistd.h>

#endif


#include <vulkan.h>
#include <vector>
#include "PhysicalDevice.h"

class Instance {
public:
    
    // Reference to the Vulkan Instance
    VkInstance vkInstance;
    
    // All the capable PhysicalDevices of the machine
    std::vector<PhysicalDevice> physicalDevices;

    /**
    Creates a full vkInstance and enumerates and stores the PhysicalDevices
    **/
    Result Init();

};

#endif