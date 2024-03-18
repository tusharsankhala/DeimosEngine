#include "VK/Base/Instance.h"

namespace Engine_VK
{
	bool CreateInstance( const char* pAppName, const char* pEngineName, VkInstance* pVulkanInstance, VkPhysicalDevice* pPhysicalDevice, InstanceProperties* pIp )
	{
		return true;
	}

	VkInstance CreateInstance( VkApplicationInfo app_info, InstanceProperties* pIp )
	{
		VkInstance instance;

		return instance;
	}

	void DestroyInstance( VkInstance instance )
	{
		vkDestroyInstance( instance, nullptr );
	}
}