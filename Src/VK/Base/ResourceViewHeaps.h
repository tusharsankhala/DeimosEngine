#pragma once

#include "VK/Base/Device.h"

namespace Engine_VK
{
	// This class will create a Descriptor pool and allow allocating, freeing and 
	// initializing Descriptor Set Layout(DSL) from this pool.

	class ResourceViewHeaps
	{
	public:
		void				OnCreate( Device* pDevice, uint32_t cbvDescriptorCount, uint32_t srvDescriptorCount, uint32_t uavDescriptorCount, uint32_t samplerDescriptorCount );
		void				OnDestroy();

	private:
		Device*				m_pDevice;
		VkDescriptorPool	m_descriptorPool;
		std::mutex			m_mutex;
		int					m_allocatorDescriptorCount = 0;
	};
}