
#include "Device.h"
#include "VK/Base/Instance.h"
#include "VK/Base/InstanceProperties.h"
#include "VK/Base/DeviceProperties.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Extensions/ExtFreeSyncHDR.h"
#include "VK/Extensions/ExtFp16.h"
#include "VK/Extensions/ExtRayTracing.h"
#include "VK/Extensions/ExtVRS.h"
#include "VK/Extensions/ExtValidation.h"
#include "Common/Misc/Misc.h"

#ifdef USE_VMA
#define VMA_IMPLEMENTATION
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#endif

namespace Engine_VK
{
	Device::Device()
	{}

	Device::~Device()
	{}

	void Device::OnCreate(const char* pAppName, const char* pEngineName, bool cpuValidationLayerEnabled, bool gpuValidationLayerEnabled, HWND hWnd)
	{
		InstanceProperties ip;
		ip.Init();
		SetEssentialInstanceExtensions( cpuValidationLayerEnabled, gpuValidationLayerEnabled, &ip );

		// Create Instance.
		VkInstance			vulkanInstance;
		VkPhysicalDevice	physicalDevice;
		CreateInstance( pAppName, pEngineName, &vulkanInstance, &physicalDevice, &ip );

		DeviceProperties dp;
		dp.Init( physicalDevice );
		SetEssentialDeviceExtensions( &dp );

		// Create Device.
		OnCreateEx( vulkanInstance, physicalDevice, hWnd, &dp );
	}

	void Device::SetEssentialInstanceExtensions( bool cpuValidationLayerEnabled, bool gpuValidationLayerEnabled, InstanceProperties* pIp )
	{
		pIp->AddInstanceExtensionName( VK_KHR_WIN32_SURFACE_EXTENSION_NAME );
		pIp->AddInstanceExtensionName( VK_KHR_SURFACE_EXTENSION_NAME );
		ExtCheckHDRInstanceExtensions( pIp );
		ExtDebugUtilsCheckInstanceExtensions( pIp );
		if( cpuValidationLayerEnabled )
		{
			ExtDebugReportCheckInstanceExtensions( pIp, gpuValidationLayerEnabled );
		}
	}

	void Device::SetEssentialDeviceExtensions(DeviceProperties* pDp)
	{
		m_usingFp16 = ExtFp16CheckExtensions( pDp );
		ExtRTCheckExtensions( pDp, m_rt10Supported, m_rt11Supported );
		ExtVRSCheckExtensions( pDp, m_vrs1Supported, m_vrs2Supported );
		ExtCheckHDRDeviceExtensions( pDp );
		ExtCheckFSEDeviceExtensions( pDp );
		ExtCheckFreeSyncHDRDeviceExtensions( pDp );
		pDp->AddDeviceExtensionName( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
		pDp->AddDeviceExtensionName(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME );
	}

	void Device::OnCreateEx(VkInstance vulkanInstance, VkPhysicalDevice physicalDevice, HWND hWnd, DeviceProperties* pDp)
	{
		VkResult res;

		m_instance			= vulkanInstance;
		m_physicalDevice	= physicalDevice;

		// Get queue/device/memory properties
		//
		uint32_t queue_family_count;
		vkGetPhysicalDeviceQueueFamilyProperties( m_physicalDevice, &queue_family_count, NULL );
		assert( queue_family_count >= 1 );

		std::vector<VkQueueFamilyProperties> queue_props;
		queue_props.resize( queue_family_count );
		vkGetPhysicalDeviceQueueFamilyProperties( m_physicalDevice, &queue_family_count, queue_props.data() );
		assert( queue_family_count >= 1 );

		vkGetPhysicalDeviceMemoryProperties( m_physicalDevice, &m_memoryProperties );
		vkGetPhysicalDeviceProperties( m_physicalDevice, &m_deviceProperties );

		// Get subgroup properties to check if subgroup operations are supported.
		//
		m_subgroupProperties.sType	= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
		m_subgroupProperties.pNext	= NULL;

		m_deviceProperties2.sType	= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		m_deviceProperties2.pNext	= &m_subgroupProperties;

		vkGetPhysicalDeviceProperties2( m_physicalDevice, &m_deviceProperties2 );

#if defined( _WIN32 )
		// Create a Win32 surface.
		// 
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType		= VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.pNext		= NULL;
		createInfo.hinstance	= NULL;
		createInfo.hwnd			= hWnd;
		res = vkCreateWin32SurfaceKHR( m_instance, &createInfo, NULL, &m_surface );
#else
	#error platform not supported
#endif

		// Find a graphics device and a queue that can present to the above surface.
		// 
		graphics_queue_family_index	= UINT32_MAX;
		present_queue_family_index	= UINT32_MAX;
		for ( uint32_t i = 0; i < queue_family_count; ++i )
		{
			if ( ( queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT ) != 0 )
			{
				if ( graphics_queue_family_index == UINT32_MAX) graphics_queue_family_index = i;

				VkBool32 supportsPresent;
				vkGetPhysicalDeviceSurfaceSupportKHR( m_physicalDevice, i, m_surface, &supportsPresent);
				if (supportsPresent == VK_TRUE)
				{
					graphics_queue_family_index = i;
					present_queue_family_index = i;
					break;
				}
			}
		}

		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		if( present_queue_family_index == UINT32_MAX )
		{
			for ( uint32_t i = 0; i < queue_family_count; ++i )
			{
				VkBool32 supportsPresent;
				vkGetPhysicalDeviceSurfaceSupportKHR( m_physicalDevice, i, m_surface, &supportsPresent );
				if( supportsPresent == VK_TRUE )
				{
					present_queue_family_index = ( uint32_t)i;
					break;
				}
			}
		}

		compute_queue_family_index = UINT32_MAX;

		for( uint32_t i = 0; i < queue_family_count; ++i )
		{
			if( ( queue_props[i].queueFlags & VK_QUEUE_COMPUTE_BIT ) != 0 )
			{
				if( compute_queue_family_index == UINT32_MAX )
					compute_queue_family_index = i;

				if (i != graphics_queue_family_index)
				{
					compute_queue_family_index = i;
					break;
				}
			}
		}

		// Prepare existing extensions names into a buffer for vkCreateDevice.
		std::vector<const char *> extension_names;
		pDp->GetExtensionNamesAndConfigs( &extension_names );
		
		// Create Device.
		//
		float queue_priorities[1] = { 0.0 };
		VkDeviceQueueCreateInfo queue_info[2] = {};
		queue_info[0].sType										= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[0].pNext										= NULL;
		queue_info[0].queueCount								= 1;
		queue_info[0].pQueuePriorities							= queue_priorities;
		queue_info[0].queueFamilyIndex							= graphics_queue_family_index;
		queue_info[1].sType										= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[1].pNext										= NULL;
		queue_info[1].queueCount								= 1;
		queue_info[1].pQueuePriorities							= queue_priorities;
		queue_info[1].queueFamilyIndex							= compute_queue_family_index;

		VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
		physicalDeviceFeatures.fillModeNonSolid					= true;
		physicalDeviceFeatures.pipelineStatisticsQuery			= true;
		physicalDeviceFeatures.fragmentStoresAndAtomics			= true;
		physicalDeviceFeatures.vertexPipelineStoresAndAtomics	= true;
		physicalDeviceFeatures.shaderImageGatherExtended		= true;
		physicalDeviceFeatures.wideLines						= true;		// Needed for drawing the lines for specific width.
		physicalDeviceFeatures.independentBlend					= true;		// Needed for having different blend for each render target.

		// Enable feature to support fp16 with subgroup operations.
		//
		VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR shaderSubgroupExtendedType = {};
		shaderSubgroupExtendedType.sType						= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES_KHR;
		shaderSubgroupExtendedType.pNext						= pDp->GetNext();	// used to be next to VkDeviceCreateInfo.
		shaderSubgroupExtendedType.shaderSubgroupExtendedTypes	= VK_TRUE;

		VkPhysicalDeviceRobustness2FeaturesEXT robustness2 = {};
		robustness2.sType										= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
		robustness2.pNext										= &shaderSubgroupExtendedType;
		robustness2.nullDescriptor								= VK_TRUE;

		// To be able to bind NULL views.
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = {};
		physicalDeviceFeatures2.sType							= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		physicalDeviceFeatures2.features						= physicalDeviceFeatures;
		physicalDeviceFeatures2.pNext							= &robustness2;

		VkDeviceCreateInfo device_info = {};
		device_info.sType										= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.pNext										= &physicalDeviceFeatures2;
		device_info.queueCreateInfoCount						= 2;
		device_info.pQueueCreateInfos							= queue_info;
		device_info.enabledExtensionCount						= (uint32_t)extension_names.size();
		device_info.ppEnabledExtensionNames						= device_info.enabledExtensionCount ? extension_names.data() : NULL;
		device_info.pEnabledFeatures							= NULL;
		res = vkCreateDevice( m_physicalDevice, &device_info, NULL, &m_device );
		assert( res == VK_SUCCESS );

#ifdef USE_VMA
		VmaAllocatorCreateInfo allocatorInfo	= {};
		allocatorInfo.physicalDevice			= GetPhysicalDevice();
		allocatorInfo.device					= GetDevice();
		allocatorInfo.instance					= m_instance;
		vmaCreateAllocator( &allocatorInfo, &m_hAllocator );
#endif // USE_VMA

		// Create queues.
		//
		vkGetDeviceQueue( m_device, graphics_queue_family_index, 0, &graphics_queue );
		if( graphics_queue_family_index == present_queue_family_index )
		{
			present_queue = graphics_queue;
		}
		else
		{
			vkGetDeviceQueue( m_device, graphics_queue_family_index, 0, &graphics_queue );
		}

		if ( compute_queue_family_index != UINT32_MAX )
		{
			vkGetDeviceQueue( m_device, graphics_queue_family_index, 0, &compute_queue );
		}

		// Init the extensios ( If they have been enabled succesfully )
		//
		ExtDebugUtilsGetProcAddresses( m_device );
		ExtGetHDRFSEFreeSyncHDRProcAddresses( m_instance, m_device );
	}


	void Device::GetDeviceInfo( std::string* deviceName, std::string* driverVersion )
	{
		#define EXTRACT(v, offset, length ) ((v>>offset) & ((1<<length)-1))
		*deviceName		= m_deviceProperties.deviceName;
		*driverVersion	= format("%i.%i.%i", EXTRACT(m_deviceProperties.driverVersion, 22, 10), EXTRACT(m_deviceProperties.driverVersion, 14, 8), EXTRACT( m_deviceProperties.driverVersion, 0, 16 ));
	}

	void Device::OnDestroy()
	{
		if( m_surface != VK_NULL_HANDLE )
		{
			vkDestroySurfaceKHR( m_instance, m_surface, NULL );
		}
	}

	void Device::CreatePipelineCache()
	{
		// Create pipeline cache.
		VkPipelineCacheCreateInfo pipelineCache;
		pipelineCache.sType				= VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		pipelineCache.pNext				= NULL;
		pipelineCache.initialDataSize	= 0;
		pipelineCache.pInitialData		= NULL;
		pipelineCache.flags				= 0;
		VkResult res = vkCreatePipelineCache( m_device, &pipelineCache, NULL, &m_pipelineCache );
		assert( res == VK_SUCCESS );
	}

	void Device::DestroyPipelineCache()
	{
		vkDestroyPipelineCache( m_device, m_pipelineCache, NULL );
	}

	VkPipelineCache Device::GetPipelineCache()
	{
		return m_pipelineCache;
	}

	void Device::GPUFlush()
	{
		vkDeviceWaitIdle( m_device );
	}

	bool memory_type_from_properties( VkPhysicalDeviceMemoryProperties& memory_properties, uint32_t typeBits, VkFlags requirement_mask, uint32_t* typeIndex )
	{
		// Search memtypes to find first index with those properties.
		for( uint32_t i=0; i < memory_properties.memoryTypeCount; ++i )
		{
			if(( typeBits & 1 ) == 1 )
			{
				// Type is availabe, does it match user properties?
				if (( memory_properties.memoryTypes[i].propertyFlags & requirement_mask ) == requirement_mask )
				{
					*typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}

		// No memory types matched, return failure.
		return false;
	}
}