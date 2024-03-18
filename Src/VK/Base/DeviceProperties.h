#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Engine_VK
{
	class DeviceProperties
	{
	private:
		VkPhysicalDevice					m_physicalDevice;

		std::vector<const char *>			m_device_extension_names;
		std::vector<VkExtensionProperties>	m_deviceExtensionProperties;

		void								*m_pNext = NULL;

	public:

		VkResult			Init( VkPhysicalDevice physicalDevice );
		bool				IsExtensionPresent( const char* pExtName );
		bool				AddDeviceExtensionName( const char* deviceExtensionName );

		void*				GetNext() { return m_pNext; }
		void*				SetNewNext( void *pNext ) { m_pNext = pNext; }

		VkPhysicalDevice	GetPhysicalDevice() { return m_physicalDevice; }
		void				GetExtensionNamesAndConfigs( std::vector<const char *> *pDevice_extension_names );
	};
}
