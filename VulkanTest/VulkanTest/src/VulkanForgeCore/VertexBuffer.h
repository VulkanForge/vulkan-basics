#ifndef VULKAN_FORGE_VERTEX_BUFFER
#define VULKAN_FORGE_VERTEX_BUFFER

#include <vulkan.h>

class Device;
class VertexBuffer {
public:
   
    Device* owningDevice;

    VkBuffer vkBuffer;


};

#endif