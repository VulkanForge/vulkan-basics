#ifndef VULKAN_FORGE_RESULT
#define VULKAN_FORGE_RESULT

#include <vulkan.h>


class Result {

public:

    VkResult vulkanError;
    VfResult forgeError;
    Resolution Resolution() { 
        if (!vulkanError) return Resolution::VULKAN_ERROR;
        if (!forgeError) return Resolution::FORGE_ERROR;
        return Resolution::SUCCESS;
    };
    
    inline Result(): vulkanError(VkResult::VK_SUCCESS), forgeError(VfResult::SUCCESS) {};
};


enum Resolution { SUCCESS, VULKAN_ERROR, FORGE_ERROR };

enum VfResult {
    SUCCESS = 0,
    IMAGE_FORMAT_D16_UNORM_UNSUPPORTED,
    MEMORY_TYPE_REQUIRED_NOT_AVAILABLE,
    MEMORY_HOST_VISIBLE_BIT_NOT_AVAILABLE,
    FRAME_BUFFERS_NOT_ALLOCATED,
    GLSLANG_TO_SPIRV_FAILED
};

#endif