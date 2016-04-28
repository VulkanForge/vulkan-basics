#pragma once
#include <iostream>
#include <cstdlib>
#include <vector>
#include <SDL.h>
#include <vulkan.h>
#define APP_SHORT_NAME "vulkanForge_samples"

class VulkanCommon {
	public:
		struct VulkanForge_info {
			VkInstance inst;
			std::vector<VkPhysicalDevice> gpus;
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

		static void DestroyInstance(VulkanForge_info info, const VkAllocationCallbacks* pAllocator = NULL) {
			vkDestroyInstance(info.inst, pAllocator);
		}

		static VkResult CheckDevices(VulkanForge_info& info) {
			uint32_t gpu_count = 1;
			VkResult res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, NULL);

			info.gpus.resize(gpu_count);
			res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, info.gpus.data());
			return res;
		}
};
