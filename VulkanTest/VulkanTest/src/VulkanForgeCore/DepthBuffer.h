#ifndef VULKAN_FORGE_DEPTH_BUFFER
#define VULKAN_FORGE_DEPTH_BUFFER

#include <vulkan.h>

class Device;
class DepthBuffer {

public:

    Device* owningDevice;

    VkBuffer vkBuffer;

};


#endif
