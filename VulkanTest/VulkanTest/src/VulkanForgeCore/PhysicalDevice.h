#ifndef VULKAN_FORGE_PHYSICAL_DEVICE_H_
#define VULKAN_FORGE_PHYSICAL_DEVICE_H_

#include "Device.h"
#include <vulkan.h>

class Instance;
class PhysicalDevice {
public:

    // The owner VulkanForge Instance
    Instance* owningInstance;

    // Reference to the Vulkan PhysicalDevice
    VkPhysicalDevice physicalDevice;

    // The created device
    Device device;

    VkPhysicalDeviceMemoryProperties physicalDeviceProperties;

    /**
    Creates a PhysicalDevice, creating and storing the Device in the process in addition to the VkPhysicalDevice
    **/
    Result Init();

//private:

    // VkDeviceQueueCreateInfo creationInfo;    // Useles??
    
};

#endif