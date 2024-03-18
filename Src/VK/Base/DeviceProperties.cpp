#include <Common/stdafx.h>
#include "DeviceProperties.h"
#include <Common/Misc/Misc.h>

namespace Engine_VK
{
	bool DeviceProperties::IsExtensionPresent( const char* pExtName )
	{
		return std::find_if(
		m_deviceExtensionProperties.begin(),
		m_deviceExtensionProperties.end(),
		[pExtName](const VkExtensionProperties& extensionProps) -> bool
		{
			return strcmp( extensionProps.extensionName, pExtName ) == 0; 
		}) != m_deviceExtensionProperties.end();
	}

	VkResult DeviceProperties::Init( VkPhysicalDevice physicalDevice )
	{
		m_physicalDevice = physicalDevice;

		// Enumerate device extensions.
		//
		uint32_t extensionCount;
		VkResult res = vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionCount, NULL );
		m_deviceExtensionProperties.resize( extensionCount );
		res = vkEnumerateDeviceExtensionProperties( physicalDevice, nullptr, &extensionCount, m_deviceExtensionProperties.data() );

		return res;
	}

	bool DeviceProperties::AddDeviceExtensionName( const char* deviceExtensionName )
	{
		if ( IsExtensionPresent( deviceExtensionName ) )
		{
			m_device_extension_names.push_back( deviceExtensionName );
			return true;
		}

		Trace( "The device Extension '%s' has not been found", deviceExtensionName );
		return false;
	}

	void DeviceProperties::GetExtensionNamesAndConfigs( std::vector<const char*>* pDevice_extension_names )
	{
		for( auto& name : m_device_extension_names )
			pDevice_extension_names->push_back( name );
	}
}