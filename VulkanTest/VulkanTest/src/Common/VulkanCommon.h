#ifndef VULKAN_COMMON_H_
#define VULKAN_COMMON_H_


#define APP_SHORT_NAME "vulkanForge_samples"
#define COMMANDBUFFER_FENCE_TIMEOUT 100 * 1000 * 1000
#define NUM_SAMPLES VK_SAMPLE_COUNT_1_BIT


#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX /* Don't let Windows define min() or max() */
#define APP_NAME_STR_LEN 80
#define NUM_DESCRIPTOR_SETS 1

#define NUM_VIEWPORTS 1
#define NUM_SCISSORS 1
#define FENCE_TIMEOUT 100000000

#else // _WIN32
#include <unistd.h>
#endif // _WIN32



#include <vulkan.h>
#include <vector>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <SPIRV/GlslangToSpv.h>

#include "../Data/cube_data.h"

class VulkanCommon {
public:
    
    enum VulkanForge_Result {
        SUCCESS = 0,
        IMAGE_FORMAT_D16_UNORM_UNSUPPORTED,
        MEMORY_TYPE_REQUIRED_NOT_AVAILABLE,
        MEMORY_HOST_VISIBLE_BIT_NOT_AVAILABLE,
        FRAME_BUFFERS_NOT_ALLOCATED,
        GLSLANG_TO_SPIRV_FAILED
    };

    struct VulkanForge_outcome {
        VkResult vkResult;
        VulkanForge_Result vfResult;
    };

    struct VulkanForge_swapchainBuffer {
        VkImage swapchainImage;
        VkImageView swapchainView;
    };

    struct VulkanForge_layerProperties {
        VkLayerProperties properties;
        std::vector<VkExtensionProperties> extensions;
    };

    struct VulkanForge_depth {
        VkFormat format;

        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    };

    struct VulkanForge_uniform {
        VkBuffer buffer;
        VkDeviceMemory deviceMemory;
        VkDescriptorBufferInfo bufferInfo;
    };

    typedef VulkanForge_uniform VulkanForge_vertexBuffer;

    struct VulkanForge_info {
        uint32_t width;
        uint32_t height;

        std::vector<const char *> instanceLayerNames;
        std::vector<const char *> instanceExtensionNames;
        std::vector<VulkanForge_layerProperties> instanceLayerProperties;
        VkInstance inst;
        
        std::vector<const char *> deviceLayerNames;
        std::vector<const char *> deviceExtensionNames;
        std::vector<VkPhysicalDevice> gpus;
        VkDevice device;
        VkPhysicalDeviceMemoryProperties memoryProperties;
        VkPhysicalDeviceProperties gpuProperties;

        uint32_t queueCount;
        VkDeviceQueueCreateInfo queueInfo;
        VkQueue queue;
        std::vector<VkQueueFamilyProperties> queueProps;
        uint32_t graphicsQueueFamilyIndex;

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;

        HINSTANCE connection;
        HWND window;
        VkSurfaceKHR surface;
        VkFormat format;

        VkSwapchainKHR swapchain;
        uint32_t swapchainImageCount;
        std::vector<VulkanForge_swapchainBuffer> swapchainBuffers;

        VulkanForge_depth depth;

        VulkanForge_uniform uniform;
        std::vector<VkDescriptorSetLayout> descriptorLayout;
        VkPipelineLayout pipelineLayout;

        VkRenderPass renderPass;

        VkFramebuffer* frameBuffers;

        VulkanForge_vertexBuffer vertexBuffer;

        VkVertexInputBindingDescription viBinding;
        VkVertexInputAttributeDescription viAttribs[2];

        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSet;

        VkPipelineCache pipelineCache;
        VkPipeline pipeline;

        uint32_t currentBuffer;

        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        VkViewport viewport;
        VkRect2D scissor;

        glm::mat4 Projection;
        glm::mat4 View;
        glm::mat4 Model;
        glm::mat4 Clip;
        glm::mat4 MVP;
    };

private:

    struct swapChainCreationAuxiliar {
        VkSurfaceCapabilitiesKHR surfCapabilities;
        uint32_t presentModeCount;
        VkPresentModeKHR *presentModes;
    };

    //Utils
    static void InitResources(TBuiltInResource &Resources);

    static EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type);

    static bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *pshader, std::vector<unsigned int> &spirv);

    //

    static VkResult InitGlobalExtensionProperties(VulkanForge_layerProperties &layer_props);

    static VkResult InitializeDeviceSurface(VulkanForge_info& info, swapChainCreationAuxiliar& commonData, const VkAllocationCallbacks* pAllocator = NULL);

    static VkResult InitializeSwapChain(VulkanForge_info& info, swapChainCreationAuxiliar& commonData, const VkAllocationCallbacks* pAllocator = NULL);
    
    static void SetImageLayout( VulkanForge_info& info, 
                                VkImage image,
                                VkImageAspectFlags aspectMask,
                                VkImageLayout old_image_layout,
                                VkImageLayout new_image_layout);
    
    static bool checkMemoryTypesFromProperties(VulkanForge_info &info, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex);

    
public:

    static VkResult InitGlobalLayerProperties(VulkanForge_info& info);

    static VkResult CreateInstance(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);

    static void DestroyInstance(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);


    static VkResult CheckDevices(VulkanForge_info& info);

    static bool TryGetGraphicQueue(VulkanForge_info& info);


    static VkResult CreateDevice(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);

    static void DestroyDevice(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);


    static VkResult CreateCommandPool(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);

    static void DestroyCommandPool(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);


    static VkResult CreateCommandBuffer(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);

    static VkResult BeginCommandBuffer(VulkanForge_info& info);

    static VkResult EndCommandBuffer(VulkanForge_info& info);

    static VkResult ExecuteQueueCommandBuffer(VulkanForge_info& info);

    static void FreeCommandBuffer(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);


    static VkResult CreateSurfaceDescription(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);

    static bool TryGetGraphicAndPresentQueue(VulkanForge_info& info);

    static VkResult InitDeviceSurfaceAndSwapChain(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL);

    static VkResult PopulateSwapChainImages(VulkanForge_info& info);


    static VulkanForge_outcome CreateDepthBuffer(VulkanForge_info& info);

    static VulkanForge_outcome CreateUniformBuffer(VulkanForge_info& info);

    static VulkanForge_outcome CreatePipelineLayout(VulkanForge_info& info);

    static VulkanForge_outcome InitRenderPass(VulkanForge_info& info);

    static VulkanForge_outcome InitShaders(VulkanForge_info& info);

    static VulkanForge_outcome InitFrameBuffer(VulkanForge_info& info);

    static VulkanForge_outcome CreateVertexBuffer(VulkanForge_info& info);

    static VulkanForge_outcome AllocateDescriptorSet(VulkanForge_info& info);

    static VulkanForge_outcome InitPipeline(VulkanForge_info& info);

    static VulkanForge_outcome DrawCube(VulkanForge_info& info);
};

#endif