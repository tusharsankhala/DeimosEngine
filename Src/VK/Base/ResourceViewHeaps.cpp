#include "ResourceViewHeaps.h"

namespace Engine_VK
{
	void ResourceViewHeaps::OnCreate( Device* pDevice, uint32_t cbvDescriptorCount, uint32_t srvDescriptorCount, uint32_t uavDescriptorCount, uint32_t samplerDescriptorCount)
	{
		m_pDevice = pDevice;
		m_allocatorDescriptorCount = 0;

		const VkDescriptorPoolSize type_count[] = 
		{
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	cbvDescriptorCount },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			cbvDescriptorCount },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	srvDescriptorCount },
			{ VK_DESCRIPTOR_TYPE_SAMPLER,					samplerDescriptorCount },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			uavDescriptorCount },
		};

		VkDescriptorPoolCreateInfo descriptor_pool = {};
		descriptor_pool.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool.pNext			= NULL;
		descriptor_pool.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptor_pool.maxSets			= 8000;
		descriptor_pool.poolSizeCount	= _countof( type_count );
		descriptor_pool.pPoolSizes		= type_count;

		VkResult res = vkCreateDescriptorPool( pDevice->GetDevice(), &descriptor_pool, NULL, &m_descriptorPool );
		assert( res == VK_SUCCESS );
	}

	void ResourceViewHeaps::OnDestroy()
	{
		vkDestroyDescriptorPool( m_pDevice->GetDevice(), m_descriptorPool, NULL );
	}
}