#ifndef VULKAN_FORGE_DEVICE_H_
#define VULKAN_FORGE_DEVICE_H_

#include <vulkan.h>
#include "SwapChain.h"
#include "UniformBuffer.h"
#include "VertexBuffer.h"
#include "DepthBuffer.h"
#include "CommandBuffer.h"


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

    SwapChain CreateSwapchain();
    
};

#endif