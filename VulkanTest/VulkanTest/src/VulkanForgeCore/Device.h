#ifndef VULKAN_FORGE_DEVICE_H_
#define VULKAN_FORGE_DEVICE_H_

#include <vulkan.h>

class PhysicalDevice;

class Device {
public:
    PhysicalDevice* owningPhysicalDevice;

    VkDevice device;

    ///////// Buffers have Init() which returns the result, createXBuffer returns the class prepared to Init itself

    UniformBuffer CreateUniformBuffer();

    VertexBuffer CreateVertexBuffer();

    DepthBuffer CreateDepthBuffer();

    CommandBuffer CreateCommandBuffer();

};

#endif