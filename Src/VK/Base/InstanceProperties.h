#pragma once

#include <vulkan/vulkan.h>

namespace Engine_VK
{
	class InstanceProperties
	{
	private:
		std::vector<VkLayerProperties>		m_instanceLayerProperties;
		std::vector<VkExtensionProperties>	m_instanceExtensionProperties;

		std::vector<const char*>			m_instance_layer_names;
		std::vector<const char*>			m_instance_extension_names;
		void*								m_pNext = NULL;

	public:
		VkResult	Init();
		bool		AddInstanceLayerName( const char* instanceLayerName );
		bool		AddInstanceExtensionName( const char* instanceExtensionName );
		void*		GetNext() { return m_pNext; }
		void		SetNewNext( void* pNext ) { m_pNext = pNext; }
	
		void		GetExtensionNamesAndConfigs( std::vector<const char *> *pInstance_layer_names, std::vector<const char *> *pInstance_extensions_names );
		
	private:
		bool		IsLayerPresent( const char* pLayerName );
		bool		IsExtensionPresent( const char* pExtName );
	};
}