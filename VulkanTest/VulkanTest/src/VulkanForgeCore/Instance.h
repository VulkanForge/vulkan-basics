#ifndef VULKAN_FORGE_INSTANCE_H_
#define VULKAN_FORGE_INSTANCE_H_


#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#else // _WIN32
#include <unistd.h>

#endif

#include <iostream>
#include <vulkan.h>
#include <vector>
#include "PhysicalDevice.h"
#include "Result.h"

using namespace std;

class Instance {
private:

    vector<const char*> _instanceExtensionNames;
    vector<const char*> _instanceLayerNames;
    
    VkResult EnumeratePhysicalDevices();


public:
    
    // Reference to the Vulkan Instance
    VkInstance* vkInstance;
    
    // All the capable PhysicalDevices of the machine
    vector<PhysicalDevice> physicalDevices;

    /**
    Creates a full vkInstance and enumerates and stores the PhysicalDevices
    **/
    Result Init(const char* appName);


    void EnableSurfaceExtension();

};

#endif