#include "VulkanCommon.h"

VkResult VulkanCommon::InitGlobalExtensionProperties(VulkanForge_layerProperties &layer_props) {
	VkExtensionProperties *instance_extensions;
	uint32_t instance_extension_count;
	VkResult res;
	char *layer_name = NULL;

	layer_name = layer_props.properties.layerName;

	do {
		res = vkEnumerateInstanceExtensionProperties(
			layer_name, &instance_extension_count, NULL);
		if (res) return res;

		if (instance_extension_count == 0) {
			return VK_SUCCESS;
		}

		layer_props.extensions.resize(instance_extension_count);
		instance_extensions = layer_props.extensions.data();
		res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, instance_extensions);
	} while (res == VK_INCOMPLETE);

	return res;
}


VkResult VulkanCommon::InitGlobalLayerProperties(VulkanForge_info& info) {
	uint32_t instance_layer_count;
	VkLayerProperties *vk_props = NULL;
	VkResult res;

	/*
	* It's possible, though very rare, that the number of
	* instance layers could change. For example, installing something
	* could include new layers that the loader would pick up
	* between the initial query for the count and the
	* request for VkLayerProperties. The loader indicates that
	* by returning a VK_INCOMPLETE status and will update the
	* the count parameter.
	* The count parameter will be updated with the number of
	* entries loaded into the data pointer - in case the number
	* of layers went down or is smaller than the size given.
	*/
	do {
		res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
		if (res) return res;

		if (instance_layer_count == 0) {
			return VK_SUCCESS;
		}

		vk_props = (VkLayerProperties *)realloc(
			vk_props, instance_layer_count * sizeof(VkLayerProperties));

		res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
	} while (res == VK_INCOMPLETE);

	/*
	* Now gather the extension list for each instance layer.
	*/
	for (uint32_t i = 0; i < instance_layer_count; i++) {
		VulkanForge_layerProperties layer_props;
		layer_props.properties = vk_props[i];
		res = InitGlobalExtensionProperties(layer_props);
		if (res)
			return res;
		info.instanceLayerProperties.push_back(layer_props);
	}
	free(vk_props);

	return res;
}

bool VulkanCommon::checkMemoryTypesFromProperties(VulkanForge_info& info, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < 32; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((info.memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
				*typeIndex = i;
				return true;
			}
		}
		typeBits >>= 1;
	}
	// No memory types matched, return failure
	return false;
}


VkResult VulkanCommon::CreateInstance(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
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


	info.instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
	info.instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
	info.enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
	inst_info.enabledExtensionCount = info.instanceExtensionNames.size();
	inst_info.ppEnabledExtensionNames = &info.instanceExtensionNames[0];

	//inst_info.enabledExtensionCount = 0;
	//inst_info.ppEnabledExtensionNames = NULL;
	inst_info.enabledLayerCount = info.instanceLayerNames.size();
	inst_info.ppEnabledLayerNames = info.instanceLayerNames.size() ? info.instanceLayerNames.data() : NULL;

	return vkCreateInstance(&inst_info, pAllocator, &info.inst);
}

void VulkanCommon::DestroyInstance(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
	vkDestroyInstance(info.inst, pAllocator);
}

VkResult VulkanCommon::CheckDevices(VulkanForge_info& info) {
	uint32_t gpu_count = 1;
	VkResult res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, NULL);

	info.gpus.resize(gpu_count);
	res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, info.gpus.data());
	return res;
}

bool VulkanCommon::TryGetGraphicQueue(VulkanForge_info& info) {
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

	// POLLAS!!!! HDP!!!
	vkGetPhysicalDeviceMemoryProperties(info.gpus[0], &info.memoryProperties);
	vkGetPhysicalDeviceProperties(info.gpus[0], &info.gpuProperties);

	return found;
}

VkResult VulkanCommon::CreateDevice(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
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
	info.deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	deviceInfo.enabledExtensionCount = info.deviceExtensionNames.size();
	deviceInfo.ppEnabledExtensionNames = info.deviceExtensionNames.size() ? info.deviceExtensionNames.data() : NULL;
	deviceInfo.enabledLayerCount = info.deviceLayerNames.size();
	deviceInfo.ppEnabledLayerNames = info.deviceLayerNames.size() ? info.deviceLayerNames.data() : NULL;
	deviceInfo.pEnabledFeatures = NULL;

	return vkCreateDevice(info.gpus[0], &deviceInfo, pAllocator, &info.device);
}

void VulkanCommon::DestroyDevice(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
	vkDestroyDevice(info.device, pAllocator);
}


VkResult VulkanCommon::CreateCommandPool(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {

	/* Create a command pool to allocate our command buffer from */
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = info.graphicsQueueFamilyIndex;
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	return vkCreateCommandPool(info.device, &cmd_pool_info, pAllocator, &info.commandPool);
}

void VulkanCommon::DestroyCommandPool(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
	vkDestroyCommandPool(info.device, info.commandPool, pAllocator);
}


VkResult VulkanCommon::CreateCommandBuffer(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
	VkCommandBufferAllocateInfo cmd = {};
	cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd.pNext = NULL;
	cmd.commandPool = info.commandPool;
	cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd.commandBufferCount = 1;

	return vkAllocateCommandBuffers(info.device, &cmd, &info.commandBuffer);
}

VkResult VulkanCommon::BeginCommandBuffer(VulkanForge_info& info) {
	VkResult res;

	VkCommandBufferBeginInfo cmd_buf_info = {};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmd_buf_info.pNext = NULL;
	cmd_buf_info.flags = 0;
	cmd_buf_info.pInheritanceInfo = NULL;

	return vkBeginCommandBuffer(info.commandBuffer, &cmd_buf_info);
}

VkResult VulkanCommon::EndCommandBuffer(VulkanForge_info& info) {
	return vkEndCommandBuffer(info.commandBuffer);
}

VkResult VulkanCommon::ExecuteQueueCommandBuffer(VulkanForge_info& info) {
	VkResult res;

	/* Queue the command buffer for execution */
	const VkCommandBuffer cmd_bufs[] = { info.commandBuffer };
	VkFenceCreateInfo fenceInfo;
	VkFence drawFence;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = 0;
	vkCreateFence(info.device, &fenceInfo, NULL, &drawFence);

	VkPipelineStageFlags pipe_stage_flags =	VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkSubmitInfo submit_info[1] = {};
	submit_info[0].pNext = NULL;
	submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info[0].waitSemaphoreCount = 0;
	submit_info[0].pWaitSemaphores = NULL;
	submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
	submit_info[0].commandBufferCount = 1;
	submit_info[0].pCommandBuffers = cmd_bufs;
	submit_info[0].signalSemaphoreCount = 0;
	submit_info[0].pSignalSemaphores = NULL;

	res = vkQueueSubmit(info.queue, 1, submit_info, drawFence);
	if (res != VK_SUCCESS) return res;

	//CALCULOS COMPLEJOS

	do {
		res = vkWaitForFences(info.device, 1, &drawFence, VK_TRUE, COMMANDBUFFER_FENCE_TIMEOUT);
	} while (res == VK_TIMEOUT);

	if (res != VK_SUCCESS) return res;

	vkDestroyFence(info.device, drawFence, NULL);

	return res;
}



void VulkanCommon::FreeCommandBuffer(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
	vkFreeCommandBuffers(info.device, info.commandPool, 1, &info.commandBuffer);
}


VkResult VulkanCommon::CreateSurfaceDescription(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {
	VkResult res;
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = GetModuleHandle(NULL);// info.connection;
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

bool VulkanCommon::TryGetGraphicAndPresentQueue(VulkanForge_info& info) {

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *supportsPresent = (VkBool32 *)malloc(info.queueCount * sizeof(VkBool32));
	for (uint32_t i = 0; i < info.queueCount; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(info.gpus[0], i, info.surface, &supportsPresent[i]);
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

	// Generate error if could not find a queue that supports both a graphics and present
	info.graphicsQueueFamilyIndex = graphicsQueueNodeIndex;

	return (graphicsQueueNodeIndex != UINT32_MAX);
}

VkResult VulkanCommon::InitDeviceSurfaceAndSwapChain(VulkanForge_info& info, const VkAllocationCallbacks* pAllocator) {

	VkResult res;
	// Estructuras comunes
	swapChainCreationAuxiliar commonData;


	// Device Surface
	res = InitializeDeviceSurface(info, commonData, pAllocator);
	if (res != VK_SUCCESS) return res;

	// Swap Chain
	res = InitializeSwapChain(info, commonData, pAllocator);
	if (res != VK_SUCCESS) return res;

	free(commonData.presentModes);

	return res;
}

VkResult VulkanCommon::InitializeDeviceSurface(VulkanForge_info& info, swapChainCreationAuxiliar& commonData, const VkAllocationCallbacks* pAllocator) {

	VkResult res;
	// Get the list of VkFormats that are supported:
	uint32_t formatCount;
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface, &formatCount, NULL);
	if (res != VK_SUCCESS) return res;

	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface, &formatCount, surfFormats);
	if (res != VK_SUCCESS) return res;

	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		info.format = VK_FORMAT_B8G8R8A8_UNORM;
	} else {
		if (formatCount <= 0) return res;
		info.format = surfFormats[0].format;
	}
	free(surfFormats);


	res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info.gpus[0], info.surface, &commonData.surfCapabilities);
	if (res != VK_SUCCESS) return res;

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface, &commonData.presentModeCount, NULL);
	if (res != VK_SUCCESS) return res;


	commonData.presentModes = (VkPresentModeKHR *)malloc(commonData.presentModeCount * sizeof(VkPresentModeKHR));
	res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface, &commonData.presentModeCount, commonData.presentModes);
	if (res != VK_SUCCESS) return res;


	return res;
}


VkResult VulkanCommon::InitializeSwapChain(VulkanForge_info& info, swapChainCreationAuxiliar& commonData, const VkAllocationCallbacks* pAllocator) {

	VkResult res;

	res = vkGetPhysicalDeviceSurfacePresentModesKHR(info.gpus[0], info.surface, &commonData.presentModeCount, commonData.presentModes);
	if (res != VK_SUCCESS) return res;

	VkExtent2D swapChainExtent;
	// width and height are either both -1, or both not -1.
	if (commonData.surfCapabilities.currentExtent.width == (uint32_t)-1) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapChainExtent.width = info.width;
		swapChainExtent.height = info.height;
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapChainExtent = commonData.surfCapabilities.currentExtent;
	}

	// If mailbox mode is available, use it, as is the lowest-latency non-
	// tearing mode.  If not, try IMMEDIATE which will usually be available,
	// and is fastest (though it tears).  If not, fall back to FIFO which is
	// always available.
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < commonData.presentModeCount; i++) {
		if (commonData.presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && commonData.presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	// Determine the number of VkImage's to use in the swap chain (we desire to
	// own only 1 image at a time, besides the images being displayed and
	// queued for display):
	uint32_t desiredNumberOfSwapChainImages = commonData.surfCapabilities.minImageCount + 1;
	if (commonData.surfCapabilities.maxImageCount > 0 && desiredNumberOfSwapChainImages > commonData.surfCapabilities.maxImageCount) {
		// Application must settle for fewer images than desired:
		desiredNumberOfSwapChainImages = commonData.surfCapabilities.maxImageCount;
	}

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (commonData.surfCapabilities.supportedTransforms &	VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else {
		preTransform = commonData.surfCapabilities.currentTransform;
	}

	VkSwapchainCreateInfoKHR swap_chain = {};
	swap_chain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swap_chain.pNext = NULL;
	swap_chain.surface = info.surface;
	swap_chain.minImageCount = desiredNumberOfSwapChainImages;
	swap_chain.imageFormat = info.format;
	swap_chain.imageExtent.width = swapChainExtent.width;
	swap_chain.imageExtent.height = swapChainExtent.height;
	swap_chain.preTransform = preTransform;
	swap_chain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swap_chain.imageArrayLayers = 1;
	swap_chain.presentMode = swapchainPresentMode;
	swap_chain.oldSwapchain = VK_NULL_HANDLE;
	swap_chain.clipped = true;
	swap_chain.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swap_chain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swap_chain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swap_chain.queueFamilyIndexCount = 0;
	swap_chain.pQueueFamilyIndices = NULL;
	
	res = vkCreateSwapchainKHR(info.device, &swap_chain, NULL, &info.swapchain);
	if (res != VK_SUCCESS) return res;

	res = vkGetSwapchainImagesKHR(info.device, info.swapchain, &info.swapchainImageCount, NULL);
	if (res != VK_SUCCESS) return res;

	VkImage *swapchainImages = (VkImage *)malloc(info.swapchainImageCount * sizeof(VkImage));

	if (swapchainImages == NULL) return res;
	//assert(swapchainImages);

	res = vkGetSwapchainImagesKHR(info.device, info.swapchain,	&info.swapchainImageCount, swapchainImages);
	if (res != VK_SUCCESS) return res;

	info.swapchainBuffers.resize(info.swapchainImageCount);

	for (uint32_t i = 0; i < info.swapchainImageCount; i++) {
		info.swapchainBuffers[i].swapchainImage = swapchainImages[i];
	}
	free(swapchainImages);

	return res;
}

VkResult VulkanCommon::PopulateSwapChainImages(VulkanForge_info& info) {

	VkResult res = VK_SUCCESS;

	vkGetDeviceQueue(info.device, info.graphicsQueueFamilyIndex, 0,	&info.queue);

	for (uint32_t i = 0; i < info.swapchainImageCount; i++) {
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.format = info.format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.flags = 0;

		SetImageLayout(  info,
						 info.swapchainBuffers[i].swapchainImage, 
						 VK_IMAGE_ASPECT_COLOR_BIT,
						 VK_IMAGE_LAYOUT_UNDEFINED,
						 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		color_image_view.image = info.swapchainBuffers[i].swapchainImage;

		res = vkCreateImageView(info.device, &color_image_view, NULL, &info.swapchainBuffers[i].swapchainView);
		if (res != VK_SUCCESS) return res;
	}

	return res;
}


void VulkanCommon::SetImageLayout(  VulkanForge_info& info, 
									VkImage image,
									VkImageAspectFlags aspectMask,
									VkImageLayout old_image_layout,
									VkImageLayout new_image_layout) {

	VkImageMemoryBarrier image_memory_barrier = {};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.pNext = NULL;
	image_memory_barrier.srcAccessMask = 0;
	image_memory_barrier.dstAccessMask = 0;
	image_memory_barrier.oldLayout = old_image_layout;
	image_memory_barrier.newLayout = new_image_layout;
	image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // FIXME: ESTO HUELE A FIXME
	image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_memory_barrier.image = image;
	image_memory_barrier.subresourceRange.aspectMask = aspectMask;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;

	if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}

	if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED) {
		image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}

	if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}

	VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier(info.commandBuffer, src_stages, dest_stages, 0, 0, 
		NULL, 0, NULL, 1, &image_memory_barrier); // FXME: ESTO TAMBIEN HUELE...
}


VulkanCommon::VulkanForge_outcome VulkanCommon::CreateDepthBuffer(VulkanForge_info& info) {
	VulkanCommon::VulkanForge_outcome outcome = {VkResult::VK_SUCCESS, VulkanCommon::VulkanForge_Result::SUCCESS};

	VkImageCreateInfo image_info = {};
	const VkFormat depth_format = VK_FORMAT_D16_UNORM;
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(info.gpus[0], depth_format, &props);
	if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_LINEAR;
	}
	else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else {
		/* Try other depth formats? */
		outcome.vfResult = VulkanCommon::VulkanForge_Result::IMAGE_FORMAT_D16_UNORM_UNSUPPORTED;
		return outcome;
	}

	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.pNext = NULL;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = depth_format;
	image_info.extent.width = info.width;
	image_info.extent.height = info.height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = NUM_SAMPLES;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	image_info.queueFamilyIndexCount = 0;
	image_info.pQueueFamilyIndices = NULL;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.pNext = NULL;
	view_info.image = VK_NULL_HANDLE;
	view_info.format = depth_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_R;
	view_info.components.g = VK_COMPONENT_SWIZZLE_G;
	view_info.components.b = VK_COMPONENT_SWIZZLE_B;
	view_info.components.a = VK_COMPONENT_SWIZZLE_A;
	view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.flags = 0;

	VkMemoryRequirements mem_reqs;

	info.depth.format = depth_format;

	/* Create image */
	outcome.vkResult = vkCreateImage(info.device, &image_info, NULL, &info.depth.image);
	if (outcome.vkResult) return outcome;

	vkGetImageMemoryRequirements(info.device, info.depth.image, &mem_reqs);

	mem_alloc.allocationSize = mem_reqs.size;
	/* Use the memory properties to determine the type of memory required */
	if (!checkMemoryTypesFromProperties(info, mem_reqs.memoryTypeBits, 0, /* No Requirements */ &mem_alloc.memoryTypeIndex)) {
		outcome.vfResult = VulkanCommon::VulkanForge_Result::MEMORY_TYPE_REQUIRED_NOT_AVAILABLE;
		return outcome;
	}
	
	/* Allocate memory */
	outcome.vkResult = vkAllocateMemory(info.device, &mem_alloc, NULL, &info.depth.mem);
	if (outcome.vkResult) return outcome;

	/* Bind memory */
	outcome.vkResult = vkBindImageMemory(info.device, info.depth.image, info.depth.mem, 0);
	if (outcome.vkResult) return outcome;

	/* Set the image layout to depth stencil optimal */
	SetImageLayout(info, info.depth.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	
	/* Create image view */
	view_info.image = info.depth.image;
	outcome.vkResult = vkCreateImageView(info.device, &view_info, NULL, &info.depth.view);

	return outcome;
}


VulkanCommon::VulkanForge_outcome VulkanCommon::CreateUniformBuffer(VulkanForge_info& info) {
	VulkanCommon::VulkanForge_outcome outcome = { VkResult::VK_SUCCESS, VulkanCommon::VulkanForge_Result::SUCCESS };
	
	info.Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	info.View = glm::lookAt(
		glm::vec3(0, 3, 10), // Camera is at (0,3,10), in World Space
		glm::vec3(0, 0, 0),  // and looks at the origin
		glm::vec3(0, -1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	info.Model = glm::mat4(1.0f);
	// Vulkan clip space has inverted Y and half Z.
	info.Clip = glm::mat4(	1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, -1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.5f, 0.0f,
							0.0f, 0.0f, 0.5f, 1.0f);

	info.MVP = info.Clip * info.Projection * info.View * info.Model;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = NULL;
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.size = sizeof(info.MVP);
	bufferInfo.queueFamilyIndexCount = 0;
	bufferInfo.pQueueFamilyIndices = NULL;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.flags = 0;
	outcome.vkResult = vkCreateBuffer(info.device, &bufferInfo, NULL, &info.uniform.buffer);
	if (outcome.vkResult) return outcome;


	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(info.device, info.uniform.buffer, &memoryRequirements);


	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.pNext = NULL;
	alloc_info.memoryTypeIndex = 0;

	alloc_info.allocationSize = memoryRequirements.size;
	uint32_t typeBits = memoryRequirements.memoryTypeBits;


	if (!checkMemoryTypesFromProperties(info, memoryRequirements.memoryTypeBits, 
										VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
										&alloc_info.memoryTypeIndex)) {
		outcome.vfResult = VulkanCommon::VulkanForge_Result::MEMORY_HOST_VISIBLE_BIT_NOT_AVAILABLE;
		return outcome;
	}


	outcome.vkResult = vkAllocateMemory(info.device, &alloc_info, NULL, &(info.uniform.deviceMemory));
	if (outcome.vkResult) return outcome;


	uint8_t *pData;
	outcome.vkResult = vkMapMemory(info.device, info.uniform.deviceMemory, 0, memoryRequirements.size, 0, (void **)&pData);
	if (outcome.vkResult) return outcome;

	memcpy(pData, &info.MVP, sizeof(info.MVP));

	vkUnmapMemory(info.device, info.uniform.deviceMemory);

	outcome.vkResult = vkBindBufferMemory(info.device, info.uniform.buffer, info.uniform.deviceMemory, 0);
	if (outcome.vkResult) return outcome;

	info.uniform.bufferInfo.buffer = info.uniform.buffer;
	info.uniform.bufferInfo.offset = 0;
	info.uniform.bufferInfo.range = sizeof(info.MVP);

	return outcome;
}


VulkanCommon::VulkanForge_outcome VulkanCommon::CreatePipelineLayout(VulkanForge_info& info) {

	VulkanForge_outcome outcome = { VkResult::VK_SUCCESS, VulkanCommon::VulkanForge_Result::SUCCESS };

	VkDescriptorSetLayoutBinding layout_binding = {};
	layout_binding.binding = 0;
	layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layout_binding.descriptorCount = 1;
	layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layout_binding.pImmutableSamplers = NULL;

	/* Next take layout bindings and use them to create a descriptor set layout
	*/
	VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
	descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_layout.pNext = NULL;
	descriptor_layout.bindingCount = 1;
	descriptor_layout.pBindings = &layout_binding;

	info.descriptorLayout.resize(NUM_DESCRIPTOR_SETS);
	outcome.vkResult = vkCreateDescriptorSetLayout(info.device, &descriptor_layout, NULL, info.descriptorLayout.data());
	
	//assert(res == VK_SUCCESS);
	if (outcome.vkResult) return outcome;


	/* Now use the descriptor layout to create a pipeline layout */
	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = NULL;
	pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	pPipelineLayoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
	pPipelineLayoutCreateInfo.pSetLayouts = info.descriptorLayout.data();

	outcome.vkResult = vkCreatePipelineLayout(info.device, &pPipelineLayoutCreateInfo, NULL, &info.pipelineLayout);
	//assert(res == VK_SUCCESS);
	
	if (outcome.vkResult) return outcome;	
	
	/***
	*** If Texture ***

	 if (use_texture) {
        layout_bindings[1].binding = 1;
        layout_bindings[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layout_bindings[1].descriptorCount = 1;
        layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layout_bindings[1].pImmutableSamplers = NULL;
    }
	
	    descriptor_layout.bindingCount = use_texture ? 2 : 1;
	***
	***
	*/
	
	/* VULKAN_KEY_END */
	/*
	for (int i = 0; i < NUM_DESCRIPTOR_SETS; i++)
		vkDestroyDescriptorSetLayout(info.device, info.desc_layout[i], NULL);
	vkDestroyPipelineLayout(info.device, info.pipeline_layout, NULL);
	*/
	
	//destroy_device(info);
	//destroy_instance(info);
}