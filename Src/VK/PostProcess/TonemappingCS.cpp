#include "Common/stdafx.h"
#include "VK/Base/DynamicBufferRing.h"
#include "VK/Base/StaticBufferPool.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Base/Helper.h"

#include "TonemappingCS.h"

namespace Engine_VK
{
	void ToneMappingCS::OnCreate(Device* pDevice, ResourceViewHeaps* pResourceViewHeaps, DynamicBufferRing* pDynamicBufferRing)
	{
		m_pDevice = pDevice;
		m_pDynamicBufferRing = pDynamicBufferRing;
		m_pResourceViewHeaps = pResourceViewHeaps;

		std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
		layoutBindings[0].binding = 0;
		layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		layoutBindings[0].descriptorCount = 1;
		layoutBindings[0].pImmutableSamplers = NULL;

		layoutBindings[1].binding = 1;
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutBindings[1].pImmutableSamplers = NULL;

		m_pResourceViewHeaps->CreateDescriptorSetLayout(&layoutBindings, &m_descriptorSetLayout);

		m_toneMapping.OnCreate(m_pDevice, "ToneMappingCS.glsl", "main", "", m_descriptorSetLayout, 8, 8, 1, NULL);

		m_descriptorIndex = 0;
		for (int i = 0; i < s_descriptorBuffers; ++i)
		{
			m_pResourceViewHeaps->AllocDescriptor(m_descriptorSetLayout, &m_descriptorSet[i]);
		}
	}


	void ToneMappingCS::OnDestroy()
	{
		m_toneMapping.OnDestroy();

		for (int i = 0; i < s_descriptorBuffers; ++i)
		{
			m_pResourceViewHeaps->FreeDescriptor(m_descriptorSet[i]);
		}

		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_descriptorSetLayout, NULL);
	}

	void ToneMappingCS::Draw(VkCommandBuffer cmd_buf, VkImageView HDRSRV, float exposure, int tonemapper, int width, int height)
	{
		SetPerfMarkerBegin(cmd_buf, "ToneMappingCS");

		VkDescriptorBufferInfo	cbToneMappingHandle;
		TonemappingConsts*		pToneMapping;
		m_pDynamicBufferRing->AllocConstantBuffer(sizeof(TonemappingConsts), (void**)&pToneMapping, &cbToneMappingHandle);
		pToneMapping->exposure = exposure;
		pToneMapping->toneMapper = tonemapper;

		// We'll be modifying thee descriptor set(DS), to prevent writing on a DS that is in use we
		// need to tod some basic buffering. Just to keep it safe and simple we'll have 10 buffers.
		VkDescriptorSet descriptorSet = m_descriptorSet[m_descriptorIndex];
		m_descriptorIndex = (m_descriptorIndex + 1) % s_descriptorBuffers;

		// Modify Descriptor set.
		SetDescriptorSet(m_pDevice->GetDevice(), 1, HDRSRV, descriptorSet);
		m_pDynamicBufferRing->SetDescriptorSet(0, sizeof(TonemappingConsts), descriptorSet);

		// Draw.
		m_toneMapping.Draw(cmd_buf, &cbToneMappingHandle, descriptorSet, (width + 7) / 8, (height + 7) / 8, 1);

		SetPerfMarkerEnd(cmd_buf);
	}
}