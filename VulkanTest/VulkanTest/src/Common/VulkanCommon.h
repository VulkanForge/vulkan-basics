#ifndef VULKAN_COMMON_H_
#define VULKAN_COMMON_H_


#define APP_SHORT_NAME "vulkanForge_samples"
#define COMMANDBUFFER_FENCE_TIMEOUT 100 * 1000 * 1000


#ifdef _WIN32
#pragma comment(linker, "/subsystem:console")
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX /* Don't let Windows define min() or max() */
#define APP_NAME_STR_LEN 80
#else // _WIN32
#include <unistd.h>
#endif // _WIN32



#include <vulkan.h>
#include <vector>

class VulkanCommon {
public:
	struct VulkanForge_swapchainBuffer {
		VkImage swapchainImage;
		VkImageView swapchainView;
	};

	struct VulkanForge_info {
		uint32_t width;
		uint32_t height;

		VkInstance inst;
		std::vector<const char *> enabledExtensions;
		std::vector<VkPhysicalDevice> gpus;

		uint32_t queueCount;
		VkDeviceQueueCreateInfo queueInfo;
		VkQueue queue;
		std::vector<VkQueueFamilyProperties> queueProps;
		uint32_t graphicsQueueFamilyIndex;

		VkDevice device;

		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;

		HINSTANCE connection;
		HWND window;
		VkSurfaceKHR surface;
		VkFormat format;

		VkSwapchainKHR swapchain;
		uint32_t swapchainImageCount;
		std::vector<VulkanForge_swapchainBuffer> swapchainBuffers;
	};

private:

	struct swapChainCreationAuxiliar {
		VkSurfaceCapabilitiesKHR surfCapabilities;
		uint32_t presentModeCount;
		VkPresentModeKHR *presentModes;
	};

	static VkResult InitializeDeviceSurface(VulkanForge_info& info, swapChainCreationAuxiliar& commonData, const VkAllocationCallbacks* pAllocator = NULL);

	static VkResult InitializeSwapChain(VulkanForge_info& info, swapChainCreationAuxiliar& commonData, const VkAllocationCallbacks* pAllocator = NULL);
	
	static void SetImageLayout( VulkanForge_info& info, 
								VkImage image,
								VkImageAspectFlags aspectMask,
								VkImageLayout old_image_layout,
								VkImageLayout new_image_layout);

public:

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

	static VkResult VulkanCommon::PopulateSwapChainImages(VulkanForge_info& info);
};

#endif