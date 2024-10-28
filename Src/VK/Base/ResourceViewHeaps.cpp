#include "Common/stdafx.h"
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

	bool ResourceViewHeaps::AllocDescriptor( VkDescriptorSetLayout descSetLayout, VkDescriptorSet* pDescriptorSet )
	{
		std::lock_guard< std::mutex > lock( m_mutex );

		VkDescriptorSetAllocateInfo alloc_info;
		alloc_info.sType				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.pNext				= NULL;
		alloc_info.descriptorPool		= m_descriptorPool;
		alloc_info.descriptorSetCount	= 1;
		alloc_info.pSetLayouts			= &descSetLayout;

		VkResult res = vkAllocateDescriptorSets( m_pDevice->GetDevice(), &alloc_info, pDescriptorSet );
		assert( res == VK_SUCCESS );

		m_allocatorDescriptorCount++;

		return res == VK_SUCCESS;
	}

	bool ResourceViewHeaps::AllocDescriptor(int size, const VkSampler* pSamplers, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
	{
		return false;
	}

	bool ResourceViewHeaps::AllocDescriptor(std::vector<uint32_t>& descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
	{
		return false;
	}

	bool ResourceViewHeaps::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout)
	{
		return false;
	}

	bool ResourceViewHeaps::CreateDescriptorSetLayoutAndAllocDescriptor(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescSet)
	{
		return false;
	}

	void ResourceViewHeaps::FreeDescriptor(VkDescriptorSet descriptorSet)
	{
		m_allocatorDescriptorCount--;
		vkFreeDescriptorSets( m_pDevice->GetDevice(), m_descriptorPool, 1, &descriptorSet );
	}
}