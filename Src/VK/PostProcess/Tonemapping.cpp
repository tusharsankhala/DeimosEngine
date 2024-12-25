#include "Tonemapping.h"
#include "VK/Base/DynamicBufferRing.h"
#include "VK/Base/StaticBufferPool.h"
#include "VK/Base/Helper.h"
#include "VK/Extensions/ExtDebugUtils.h"

namespace Engine_VK
{
	void ToneMapping::OnCreate(Device* pDevice, VkRenderPass renderPass, ResourceViewHeaps* pResourceViewHeaps, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing, uint32_t srvTableSize, const char* shaderSource)
	{
		m_pDevice = pDevice;
		m_pDynamicBufferRing = pDynamicBufferRing;
		m_pResourceViewHeaps = pResourceViewHeaps;

		{
			VkSamplerCreateInfo info = {};
			info.sType			= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter		= VK_FILTER_LINEAR;
			info.minFilter		= VK_FILTER_LINEAR;
			info.mipmapMode		= VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.addressModeU	= VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV	= VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW	= VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.minLod			= -1000;
			info.maxLod			= -1000;
			info.maxAnisotropy	= 1.0f;
			VkResult res = vkCreateSampler(m_pDevice->GetDevice(), &info, NULL, &m_sampler);
			assert(res == VK_SUCCESS);
		}

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[0].pImmutableSamplers = NULL;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		layoutBindings[1].descriptorCount = srvTableSize;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = NULL;

		m_pResourceViewHeaps->CreateDescriptorSetLayout(&layoutBindings, &m_descriptorSetLayout);

		m_toneMapping.OnCreate(m_pDevice, renderPass, shaderSource, "main", "", pStaticBufferPool, pDynamicBufferRing, m_descriptorSetLayout, NULL, VK_SAMPLE_COUNT_1_BIT);

		m_descriptorIndex = 0;
		for (int i = 0; i < s_descriptorBuffers; ++i)
		{
			m_pResourceViewHeaps->AllocDescriptor(m_descriptorSetLayout, &m_descriptorSet[i]);
		}
	}

	void ToneMapping::OnDestroy()
	{
		m_toneMapping.OnDestroy();

		for (int i = 0; i < s_descriptorBuffers; ++i)
		{
			m_pResourceViewHeaps->FreeDescriptor(m_descriptorSet[i]);
		}

		vkDestroySampler(m_pDevice->GetDevice(), m_sampler, nullptr);

		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_descriptorSetLayout, NULL);
	}

	void ToneMapping::UpdatePipeline(VkRenderPass renderPass)
	{
		m_toneMapping.UpdatePipeline(renderPass, NULL, VK_SAMPLE_COUNT_1_BIT);
	}

	void ToneMapping::Draw(VkCommandBuffer cmd_buf, VkImageView HDRSRV, float exposure, int tonemapper, int width, int height)
	{
		SetPerfMarkerBegin(cmd_buf, "ToneMappingCS");

		VkDescriptorBufferInfo	cbToneMappingHandle;
		TonemappingConsts* pToneMapping;
		m_pDynamicBufferRing->AllocConstantBuffer(sizeof(TonemappingConsts), (void**)&pToneMapping, &cbToneMappingHandle);
		pToneMapping->exposure = exposure;
		pToneMapping->toneMapper = tonemapper;

		// We'll be modifying thee descriptor set(DS), to prevent writing on a DS that is in use we
		// need to tod some basic buffering. Just to keep it safe and simple we'll have 10 buffers.
		VkDescriptorSet descriptorSet = m_descriptorSet[m_descriptorIndex];
		m_descriptorIndex = (m_descriptorIndex + 1) % s_descriptorBuffers;

		// Modify Descriptor set.
		SetDescriptorSet(m_pDevice->GetDevice(), 1, HDRSRV, &m_sampler, descriptorSet);
		m_pDynamicBufferRing->SetDescriptorSet(0, sizeof(TonemappingConsts), descriptorSet);

		// Draw.
		m_toneMapping.Draw(cmd_buf, &cbToneMappingHandle, descriptorSet);

		SetPerfMarkerEnd(cmd_buf);
	}
}