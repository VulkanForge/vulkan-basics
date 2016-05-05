#pragma once
#include <iostream>
#include <cstdlib>
#include <vector>
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_video.h>
#define APP_SHORT_NAME "vulkanForge_samples"

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

class VulkanCommon {
	public:
		struct VulkanForge_info {
			uint32_t width;
			uint32_t height;

			VkInstance inst;
			std::vector<VkPhysicalDevice> gpus;

			uint32_t queueCount;
			VkDeviceQueueCreateInfo queueInfo;
			std::vector<VkQueueFamilyProperties> queueProps;
			uint32_t graphicsQueueFamilyIndex;

			VkDevice device;

			VkCommandPool commandPool;
			VkCommandBuffer commandBuffer;

			HINSTANCE connection;
			HWND window;
			VkSurfaceKHR surface;
			VkFormat format;

			uint32_t presentModeCount;
			VkSurfaceCapabilitiesKHR surfCapabilities;
		};

		static VkResult CreateInstance(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			// initialize the VkApplicationInfo structure
			VkApplicationInfo app_info = {};
			app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			app_info.pNext = NULL;
			app_info.pApplicationName = APP_SHORT_NAME;
			app_info.applicationVersion = 1;
			app_info.pEngineName = APP_SHORT_NAME;
			app_info.engineVersion = 1;
			app_info.apiVersion = VK_API_VERSION_1_0;

			// initialize the VkInstanceCreateInfo structure
			VkInstanceCreateInfo inst_info = {};
			inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			inst_info.pNext = NULL;
			inst_info.flags = 0;
			inst_info.pApplicationInfo = &app_info;
			inst_info.enabledExtensionCount = 0;
			inst_info.ppEnabledExtensionNames = NULL;
			inst_info.enabledLayerCount = 0;
			inst_info.ppEnabledLayerNames = NULL;

			return vkCreateInstance(&inst_info, pAllocator, &info.inst);
		}

		static void DestroyInstance(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			vkDestroyInstance(info.inst, pAllocator);
		}

		static VkResult CheckDevices(VulkanForge_info& info) {
			uint32_t gpu_count = 1;
			VkResult res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, NULL);

			info.gpus.resize(gpu_count);
			res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, info.gpus.data());
			return res;
		}

		static bool TryGetGraphicQueue(VulkanForge_info& info) {
			vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0], &info.queueCount,
				NULL);
			//assert(info.queue_count >= 1);

			info.queueProps.resize(info.queueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0], &info.queueCount,
				info.queueProps.data());
			//assert(info.queue_count >= 1);

			bool found = false;
			for (unsigned int i = 0; i < info.queueCount; i++) {
				if (info.queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					info.queueInfo.queueFamilyIndex = i;
					found = true;
					break;
				}
			}
			return found;
		}

		static VkResult CreateDevice(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			float queuePriorities[1] = { 0.0 };
			info.queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.queueInfo.pNext = NULL;
			info.queueInfo.queueCount = 1;
			info.queueInfo.pQueuePriorities = queuePriorities;
			
			VkDeviceCreateInfo deviceInfo = {};
			deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceInfo.pNext = NULL;
			deviceInfo.queueCreateInfoCount = 1;
			deviceInfo.pQueueCreateInfos = &info.queueInfo;
			deviceInfo.enabledExtensionCount = 0;
			deviceInfo.ppEnabledExtensionNames = NULL;
			deviceInfo.enabledLayerCount = 0;
			deviceInfo.ppEnabledLayerNames = NULL;
			deviceInfo.pEnabledFeatures = NULL;

			return vkCreateDevice(info.gpus[0], &deviceInfo, pAllocator, &info.device);
		}

		static void DestroyDevice(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			vkDestroyDevice(info.device, pAllocator);
		}


		static VkResult CreateCommandPool(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {

			/* Create a command pool to allocate our command buffer from */
			VkCommandPoolCreateInfo cmd_pool_info = {};
			cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmd_pool_info.pNext = NULL;
			cmd_pool_info.queueFamilyIndex = info.graphicsQueueFamilyIndex;
			cmd_pool_info.flags = 0;

			return vkCreateCommandPool(info.device, &cmd_pool_info, pAllocator, &info.commandPool);
		}

		static void DestroyCommandPool(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			vkDestroyCommandPool(info.device, info.commandPool, pAllocator);
		}


		static VkResult CreateCommandBuffer(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			VkCommandBufferAllocateInfo cmd = {};
			cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd.pNext = NULL;
			cmd.commandPool = info.commandPool;
			cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd.commandBufferCount = 1;

			return vkAllocateCommandBuffers(info.device, &cmd, &info.commandBuffer);
		}

		static void FreeCommandBuffer(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			vkFreeCommandBuffers(info.device, info.commandPool, 1, &info.commandBuffer);
		}


		static VkResult CreateSurfaceDescription(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			VkResult res;
			#ifdef _WIN32
			VkWin32SurfaceCreateInfoKHR createInfo = {};
			createInfo.sType = VK_USE_PLATFORM_WIN32_KHR;
			createInfo.pNext = NULL;
			createInfo.hinstance = info.connection;
			createInfo.hwnd = info.window;
			res = vkCreateWin32SurfaceKHR(info.inst, &createInfo, pAllocator, &(info.surface));
			#else  // !_WIN32
			VkXcbSurfaceCreateInfoKHR createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
			createInfo.pNext = NULL;
			createInfo.connection = info.connection;
			createInfo.window = info.window;
			res = vkCreateXcbSurfaceKHR(info.inst, &createInfo, pAllocator, &info.surface);
			#endif // _WIN32
			return res;
		}

		static bool TryGetGraphicAndPresentQueue(VulkanForge_info& info) {

			// Iterate over each queue to learn whether it supports presenting:
			VkBool32 *supportsPresent = (VkBool32 *)malloc(info.queueCount * sizeof(VkBool32));
			for (uint32_t i = 0; i < info.queueCount; i++) {
				vkGetPhysicalDeviceSurfaceSupportKHR(info.gpus[0], i, info.surface,	&supportsPresent[i]);
			}

			// Search for a graphics queue and a present queue in the array of queue
			// families, try to find one that supports both
			uint32_t graphicsQueueNodeIndex = UINT32_MAX;
			for (uint32_t i = 0; i < info.queueCount; i++) {
				if ((info.queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
					if (supportsPresent[i] == VK_TRUE) {
						graphicsQueueNodeIndex = i;
						break;
					}
				}
			}
			free(supportsPresent);
			
			// Generate error if could not find a queue that supports both a graphics
			// and present
			info.graphicsQueueFamilyIndex = graphicsQueueNodeIndex;

			return (graphicsQueueNodeIndex != UINT32_MAX);				
		}

		static VkResult InitializeDeviceSurface(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
			VkResult res;
			// Get the list of VkFormats that are supported:
			uint32_t formatCount;
			res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface,
				&formatCount, NULL);
			//assert(res == VK_SUCCESS);
			if (res != VK_SUCCESS) return res;

			VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
			res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface, &formatCount, surfFormats);
			//assert(res == VK_SUCCESS);
			if (res != VK_SUCCESS) return res;

			// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
			// the surface has no preferred format.  Otherwise, at least one
			// supported format will be returned.
			if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
				info.format = VK_FORMAT_B8G8R8A8_UNORM;
			}
			else {
				if (formatCount <= 0) return res;
				info.format = surfFormats[0].format;
			}

			res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info.gpus[0], info.surface, &info.surfCapabilities);
			//assert(res == VK_SUCCESS);
			if (res != VK_SUCCESS) return res;

			res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface,	&info.presentModeCount, NULL);
			//assert(res == VK_SUCCESS);
			if (res != VK_SUCCESS) return res;

			VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(info.presentModeCount * sizeof(VkPresentModeKHR));

			res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface, &info.presentModeCount, presentModes);
			//assert(res == VK_SUCCESS);
			return res;
		}

		//FIXME hacerlo (metodo de 500 lineas vs varios metodos? *presentModes presentModeCount etc...)

//		static VkResult InitializeSwapChain(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator = NULL) {
//			VkExtent2D swapChainExtent;
//			// width and height are either both -1, or both not -1.
//			if (info.surfCapabilities.currentExtent.width == (uint32_t)-1) {
//				// If the surface size is undefined, the size is set to
//				// the size of the images requested.
//				swapChainExtent.width = info.width;
//				swapChainExtent.height = info.height;
//			}
//			else {
//				// If the surface size is defined, the swap chain size must match
//				swapChainExtent = info.surfCapabilities.currentExtent;
//			}
//
//			// If mailbox mode is available, use it, as is the lowest-latency non-
//			// tearing mode.  If not, try IMMEDIATE which will usually be available,
//			// and is fastest (though it tears).  If not, fall back to FIFO which is
//			// always available.
//			VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
//			for (size_t i = 0; i < info.presentModeCount; i++) {
//				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
//					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
//					break;
//				}
//				if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
//					(presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
//					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
//				}
//			}
//
//			// Determine the number of VkImage's to use in the swap chain (we desire to
//			// own only 1 image at a time, besides the images being displayed and
//			// queued for display):
//			uint32_t desiredNumberOfSwapChainImages =
//				info.surfCapabilities.minImageCount + 1;
//			if ((info.surfCapabilities.maxImageCount > 0) &&
//				(desiredNumberOfSwapChainImages > info.surfCapabilities.maxImageCount)) {
//				// Application must settle for fewer images than desired:
//				desiredNumberOfSwapChainImages = info.surfCapabilities.maxImageCount;
//			}
//
//			VkSurfaceTransformFlagBitsKHR preTransform;
//			if (info.surfCapabilities.supportedTransforms &
//				VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
//				preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
//			}
//			else {
//				preTransform = info.surfCapabilities.currentTransform;
//			}
//
//			VkSwapchainCreateInfoKHR swap_chain = {};
//			swap_chain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
//			swap_chain.pNext = NULL;
//			swap_chain.surface = info.surface;
//			swap_chain.minImageCount = desiredNumberOfSwapChainImages;
//			swap_chain.imageFormat = info.format;
//			swap_chain.imageExtent.width = swapChainExtent.width;
//			swap_chain.imageExtent.height = swapChainExtent.height;
//			swap_chain.preTransform = preTransform;
//			swap_chain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
//			swap_chain.imageArrayLayers = 1;
//			swap_chain.presentMode = swapchainPresentMode;
//			swap_chain.oldSwapchain = NULL;
//			swap_chain.clipped = true;
//			swap_chain.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//			swap_chain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//			swap_chain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
//			swap_chain.queueFamilyIndexCount = 0;
//			swap_chain.pQueueFamilyIndices = NULL;
//
//			res =
//				vkCreateSwapchainKHR(info.device, &swap_chain, NULL, &info.swap_chain);
//			assert(res == VK_SUCCESS);
//
//			res = vkGetSwapchainImagesKHR(info.device, info.swap_chain,
//				&info.swapchainImageCount, NULL);
//			assert(res == VK_SUCCESS);
//
//			VkImage *swapchainImages =
//				(VkImage *)malloc(info.swapchainImageCount * sizeof(VkImage));
//			assert(swapchainImages);
//			res = vkGetSwapchainImagesKHR(info.device, info.swap_chain,
//				&info.swapchainImageCount, swapchainImages);
//			assert(res == VK_SUCCESS);
//
//			info.buffers.resize(info.swapchainImageCount);
//
//		}
};
