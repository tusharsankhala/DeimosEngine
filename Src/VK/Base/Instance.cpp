
#include "VK/Common/stdafx.h"
#include <algorithm>
#include "VK/Base/Instance.h"
#include "VK/Base/InstanceProperties.h"
#include <vulkan/vulkan_win32.h>
#include "VK/Extensions/ExtValidation.h"

namespace Engine_VK
{
	uint32_t GetScore( VkPhysicalDevice physicalDevice)
	{
		uint32_t score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties( physicalDevice, &deviceProperties );
		// Use the feature for a more precise way to select the GPU
		switch (deviceProperties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				score += 1000;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				score += 1000;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				score += 1000;
				break;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				score += 10;
				break;
			default:
				break;
		}

		// TODO: add other constraints.

		return score;
	}

	// Select the best physical device.
	// For now, the code just gets the first discrete GPU.
	// If none is found, it default to an integrated then virtual then cpu one
	VkPhysicalDevice SelectPhysicalDevice(std::vector<VkPhysicalDevice>& physicalDevices)
	{
		assert(physicalDevices.size() > 0 && "No GPU Found");

		std::multimap< uint32_t, VkPhysicalDevice> ratings;

		for (auto it = physicalDevices.begin(); it != physicalDevices.end(); ++it )
		{
			ratings.insert(std::make_pair(GetScore(*it), *it));
		}

		return ratings.rbegin()->second;
	}

	bool CreateInstance( const char* pAppName, const char* pEngineName, VkInstance* pVulkanInstance, VkPhysicalDevice* pPhysicalDevice, InstanceProperties* pIp )
	{
		VkApplicationInfo app_info		= {};
		app_info.sType					= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext					= NULL;
		app_info.pApplicationName		= pAppName;
		app_info.applicationVersion		= 1;
		app_info.pEngineName			= pEngineName;
		app_info.engineVersion			= 1;
		app_info.apiVersion				= VK_API_VERSION_1_1;
		VkInstance instance				= CreateInstance( app_info, pIp );

		// Enumerate physical device.
		uint32_t physicalDeviceCount	= 1;
		uint32_t const req_count		= physicalDeviceCount;
		VkResult res					= vkEnumeratePhysicalDevices( instance, &physicalDeviceCount, nullptr );
		assert( physicalDeviceCount > 0 && "No GPU Found ");

		std::vector<VkPhysicalDevice> physicalDevices;
		physicalDevices.resize(physicalDeviceCount);
		res = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
		assert(res == VK_SUCCESS && physicalDeviceCount >= req_count && "Unable to enumerate physical devices");

		// Get the best available gpu.
		*pPhysicalDevice = SelectPhysicalDevice(physicalDevices);
		*pVulkanInstance = instance;

		return true;
	}

	VkInstance CreateInstance( VkApplicationInfo app_info, InstanceProperties* pIp )
	{
		VkInstance instance;

		// Prepare existing extensions and layer names into a buffer for vkCreateInstance.
		std::vector<const char*> instance_layer_names;
		std::vector<const char*> instance_extensions_names;
		pIp->GetExtensionNamesAndConfigs(&instance_layer_names, &instance_extensions_names);

		// Do create the instance.
		VkInstanceCreateInfo inst_info = {};
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pNext = pIp->GetNext();
		inst_info.flags = 0;
		inst_info.pApplicationInfo = &app_info;
		inst_info.enabledLayerCount = (uint32_t)instance_layer_names.size();
		inst_info.ppEnabledLayerNames = (uint32_t)instance_layer_names.size() ? instance_layer_names.data() : NULL;
		inst_info.enabledExtensionCount = (uint32_t)instance_extensions_names.size();
		inst_info.ppEnabledExtensionNames = instance_extensions_names.data();
		VkResult res = vkCreateInstance( &inst_info, NULL, &instance );
		assert(res == VK_SUCCESS);

		// Init the extensions ( if they have been enabled successfully ).
		//
		ExtDebugReportGetProcAddresses( instance );
		ExtDebugReportOnCreate(instance);

		return instance;
	}

	void DestroyInstance( VkInstance instance )
	{
		vkDestroyInstance( instance, nullptr );
	}
}