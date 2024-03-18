#pragma once

#include <vulkan/vulkan.h>
#include "InstanceProperties.h"

namespace Engine_VK
{
	//
	// Creates/Destroys a Vulkan instance.

	bool CreateInstance( const char* pAppName, const char *pEngineName, VkInstance *pVulkanInstance, VkPhysicalDevice *pPhysicalDevice, InstanceProperties *pIp );
	VkInstance CreateInstance( VkApplicationInfo app_info, InstanceProperties* pIp);
	void DestroyInstance( VkInstance instance );
}
