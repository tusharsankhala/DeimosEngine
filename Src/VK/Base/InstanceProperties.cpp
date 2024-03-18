
#include "InstanceProperties.h"
#include "Common/Misc/Misc.h"

namespace Engine_VK
{
	bool InstanceProperties::IsLayerPresent( const char* pLayerName )
	{
		return std::find_if(
		m_instanceLayerProperties.begin(),
		m_instanceLayerProperties.end(),
		[pLayerName]( const VkLayerProperties& layerProps	) -> bool {
			return strcmp( layerProps.layerName, pLayerName) == 0;
			}) != m_instanceLayerProperties.end();
	}

	bool InstanceProperties::IsExtensionPresent( const char* pExtName )
	{
		return std::find_if(
			m_instanceExtensionProperties.begin(),
			m_instanceExtensionProperties.end(),
			[pExtName](const VkExtensionProperties& extensionProps ) -> bool {
				return strcmp( extensionProps.extensionName, pExtName ) == 0;
			}) != m_instanceExtensionProperties.end();
	}

	VkResult InstanceProperties::Init()
	{
		// Query instance layers.
		//
		uint32_t instanceLayerPropertyCount = 0;
		VkResult res = vkEnumerateInstanceLayerProperties( &instanceLayerPropertyCount, nullptr );
		assert(res == VK_SUCCESS);
		m_instanceLayerProperties.resize( instanceLayerPropertyCount );
		
		if( instanceLayerPropertyCount > 0 )
		{
			res = vkEnumerateInstanceLayerProperties( &instanceLayerPropertyCount, m_instanceLayerProperties.data() );
			assert( res == VK_SUCCESS );
		}

		// Query instance extensions.
		//
		uint32_t instanceExtensionPropertyCount = 0;
		res = vkEnumerateInstanceExtensionProperties( nullptr, &instanceExtensionPropertyCount, nullptr);
		assert(res == VK_SUCCESS);
		m_instanceExtensionProperties.resize( instanceExtensionPropertyCount );
		
		if ( instanceExtensionPropertyCount > 0 )
		{
			res = vkEnumerateInstanceExtensionProperties( nullptr, &instanceExtensionPropertyCount, m_instanceExtensionProperties.data());
			assert( res == VK_SUCCESS );
		}

		return res;
	}

	bool InstanceProperties::AddInstanceLayerName( const char* instanceLayerName )
	{
		if( IsLayerPresent( instanceLayerName ) )
		{
			m_instance_layer_names.push_back( instanceLayerName );
			return true;
		}

		Trace("The instance layer '%s' has not been found\n", instanceLayerName );
	}

	bool InstanceProperties::AddInstanceExtensionName( const char* instanceExtensionName )
	{
		if ( IsExtensionPresent( instanceExtensionName ) )
		{
			m_instance_extension_names.push_back( instanceExtensionName );
			return true;
		}

		Trace("The instance extension '%s' has not been found\n", instanceExtensionName );
	}

	void InstanceProperties::GetExtensionNamesAndConfigs(std::vector<const char*>* pInstance_layer_names, std::vector<const char*>* pInstance_extensions_names)
	{
		for ( auto& name : m_instance_layer_names )
		{
			pInstance_layer_names->push_back( name );
		}

		for ( auto& name : m_instance_extension_names )
		{
			pInstance_extensions_names->push_back(name);
		}
	}
}