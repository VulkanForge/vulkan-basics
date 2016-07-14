#include "Instance.h"

Result Instance::Init(const char* appName) {
    
    Result result;

    // initialize the VkApplicationInfo structure
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = appName;
    app_info.applicationVersion = 1;
    app_info.pEngineName = "Vulkan Forge";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_0;



    // initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;

    inst_info.enabledExtensionCount = _instanceExtensionNames.size();
    inst_info.ppEnabledExtensionNames = &_instanceExtensionNames[0];

    //inst_info.enabledExtensionCount = 0;
    //inst_info.ppEnabledExtensionNames = NULL;
    inst_info.enabledLayerCount = _instanceLayerNames.size();
    inst_info.ppEnabledLayerNames = _instanceLayerNames.size() ? _instanceLayerNames.data() : NULL;

    result.vulkanError = vkCreateInstance(&inst_info, NULL, vkInstance);


    // Devices
    EnumeratePhysicalDevices();


    return result;
}


void Instance::EnableSurfaceExtension() {

    if (vkInstance != NULL) {
        cerr << "Extensions should be enabled before initialization" << endl;
        return;
    }

    _instanceExtensionNames.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
    _instanceExtensionNames.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
    _instanceExtensionNames.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

}

VkResult Instance::EnumeratePhysicalDevices() {
    

    return VkResult::VK_SUCCESS;
}
