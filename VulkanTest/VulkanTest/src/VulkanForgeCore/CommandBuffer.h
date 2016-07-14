#ifndef VULKAN_FORGE_COMMAND_BUFFER
#define VULKAN_FORGE_COMMAND_BUFFER

#include <vulkan.h>

class Device;
class CommandBuffer {

public:

    Device* owningDevice;

    VkBuffer vkBuffer;

};


#endif

