#ifndef VULKAN_FORGE_UNIFORM_BUFFER
#define VULKAN_FORGE_UNIFORM_BUFFER

#include <vulkan.h>

class Device;
class UniformBuffer {

public:

    Device* owningDevice;

    VkBuffer vkBuffer;

};


#endif
