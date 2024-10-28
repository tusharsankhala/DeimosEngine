#pragma once

#include "Common/stdafx.h"
#include <vulkan/vulkan.h>
#include "VK/Base/DeviceProperties.h"
#include "VK/Base/InstanceProperties.h"

#define USE_VMA

#ifdef USE_VMA
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#endif

namespace Engine_VK
{
	// A device calls with many helper functions.

	class Device
	{
	public:
		Device();
		~Device();

		void								OnCreate( const char* pAppName, const char* pEngineName, bool cpuValidationLayerEnabled, bool gpuValidationLayerEnabled, HWND hWnd );
		void								SetEssentialInstanceExtensions( bool cpuValidationLayerEnabled, bool gpuValidationLayerEnabled, InstanceProperties* pIp );
		void								SetEssentialDeviceExtensions( DeviceProperties* pDp);
		void								OnCreateEx( VkInstance vulkanInstance, VkPhysicalDevice physicalDevice, HWND hWnd, DeviceProperties *pDp );
		void								OnDestroy();
		VkDevice							GetDevice()						{ return m_device; }
		VkQueue								GetGraphicsQueue()				{ return graphics_queue; }
		uint32_t							GetGraphicsQueueFamilyIndex()	{ return present_queue_family_index; }
		VkQueue								GetPresentQueue()				{ return present_queue; }
		VkQueue								GetComputeQueue()				{ return compute_queue; }
		uint32_t							GetComputeQueueFamilyIndex()	{ return compute_queue_family_index; }
		VkPhysicalDevice					GetPhysicalDevice()				{ return m_physicalDevice; }
		VkSurfaceKHR						GetSurface()					{ return m_surface; }
		void								GetDeviceInfo( std::string* deviceName, std::string* driverVersion );

#ifdef USE_VMA
		VmaAllocator GetAllocator() { return m_hAllocator; }
#endif

		VkPhysicalDeviceMemoryProperties	GetPhysicalDeviceMemoryProperties() { return m_memoryProperties; }
		VkPhysicalDeviceProperties			GetPhysicalDeviceProperties() { return m_deviceProperties; }

		// Pipeline Cache.
		VkPipelineCache						m_pipelineCache;
		void								CreatePipelineCache();
		void								DestroyPipelineCache();
		VkPipelineCache						GetPipelineCache();

		void								CreateShaderCache() {}
		void								DestroyShaderCache() {}

		void								GPUFlush();

	private:
		VkInstance							m_instance;
		VkDevice							m_device;
		VkPhysicalDevice					m_physicalDevice;
		VkPhysicalDeviceMemoryProperties	m_memoryProperties;
		VkPhysicalDeviceProperties			m_deviceProperties;
		VkPhysicalDeviceProperties2			m_deviceProperties2;
		VkPhysicalDeviceSubgroupProperties	m_subgroupProperties;
		VkSurfaceKHR						m_surface;

		VkQueue								present_queue;
		uint32_t							present_queue_family_index;
		VkQueue								graphics_queue;
		uint32_t							graphics_queue_family_index;
		VkQueue								compute_queue;
		uint32_t							compute_queue_family_index;

		bool								m_usingValidationLayer	= false;
		bool								m_usingFp16				= false;
		bool								m_rt10Supported			= false;
		bool								m_rt11Supported			= false;
		bool								m_vrs1Supported			= false;
		bool								m_vrs2Supported			= false;

#ifdef USE_VMA
		VmaAllocator						m_hAllocator			= NULL;
#endif // USE_VMA
	};

	bool memory_type_from_properties( VkPhysicalDeviceMemoryProperties& memory_properties, uint32_t typeBits, VkFlags requirements_mask, uint32_t* type_index );
}
