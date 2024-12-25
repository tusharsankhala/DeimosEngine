
#include "VK/Common/stdafx.h"
#include "VK/Base/DynamicBufferRing.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Base/Texture.h"
#include "VK/Base/FreeSyncHDR.h"
#include "ColorConversionPS.h"
#include "Common/Misc/ColorConversion.h"

namespace Engine_VK
{
	
	void ColorConversionPS::OnCreate(Device* pDevice, VkRenderPass renderPass, ResourceViewHeaps* pResourceViewHeaps, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pConstantBufferRing)
	{
		m_pDevice = pDevice;
		m_pDynamicBufferRing = pConstantBufferRing;
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
			info.maxLod			= 1000;
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
		layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount = 1;
		layoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = NULL;

		m_pResourceViewHeaps->CreateDescriptorSetLayout(&layoutBindings, &m_descriptorSetLayout);

		m_colorConversion.OnCreate(m_pDevice, renderPass, "ColorConversionPS.glsl", "main", "", pStaticBufferPool, m_pDynamicBufferRing, m_descriptorSetLayout, NULL, VK_SAMPLE_COUNT_1_BIT);

		m_descriptorIndex = 0;
		for (int i = 0; i < s_descriptorBuffers; ++i)
		{
			m_pResourceViewHeaps->AllocDescriptor(m_descriptorSetLayout, &m_descriptorSet[i]);
		}
	}
	
	void ColorConversionPS::OnDestroy()
	{
		m_colorConversion.OnDestroy();

		for (int i = 0; i < s_descriptorBuffers; ++i)
		{
			m_pResourceViewHeaps->FreeDescriptor(m_descriptorSet[i]);
		}

		vkDestroySampler(m_pDevice->GetDevice(), m_sampler, nullptr);

		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_descriptorSetLayout, NULL);
	}
	

	void ColorConversionPS::UpdatePipeline(VkRenderPass renderPass, DisplayMode displayMode)
	{
		m_colorConversion.UpdatePipeline(renderPass, NULL, VK_SAMPLE_COUNT_1_BIT);
		m_colorConversionConsts.m_displayMode = displayMode;

		if (displayMode	!= DISPLAYMODE_SDR)
		{
			const VkHdrMetadataEXT* pHDRMetaData = fsHdrGetDisplayInfo();

			m_colorConversionConsts.m_displayMinLuminancePerNits = (float)pHDRMetaData->minLuminance / 80.0f; // RGB(1, 1, 1) maps to 80nits in scRGB.
			m_colorConversionConsts.m_displayMaxLuminancePerNits = (float)pHDRMetaData->maxLuminance / 80.0f; // This means that peak white equals RGB(m_maxLuminance/80.0f, m_maxLuminance/80.0f, m_maxLuminance/80.0f) in scRGB.

			FillDisplaySpecificPrimaries(
				pHDRMetaData->whitePoint.x, pHDRMetaData->whitePoint.y,
				pHDRMetaData->displayPrimaryRed.x, pHDRMetaData->displayPrimaryRed.y,
				pHDRMetaData->displayPrimaryGreen.x, pHDRMetaData->displayPrimaryGreen.y,
				pHDRMetaData->displayPrimaryBlue.x, pHDRMetaData->displayPrimaryBlue.y
				);
		}

		SetupGamutMapperMatrices(ColorSpace_REC709,
								 ColorSpace_Display,
								 &m_colorConversionConsts.m_contentToMonitorRecMatrix);
	}
	
	void ColorConversionPS::Draw(VkCommandBuffer cmd_buf, VkImageView HDRSRV)
	{
		SetPerfMarkerBegin(cmd_buf, "ColorConversion");

		VkDescriptorBufferInfo	cbTonemappingHandle;
		ColorConversionConsts*	pColorConversionConsts;
		m_pDynamicBufferRing->AllocConstantBuffer(sizeof(ColorConversionConsts), (void**)&pColorConversionConsts, &cbTonemappingHandle);
		*pColorConversionConsts = m_colorConversionConsts;

		// We'll be modifying the descriptor set(DS), to prevent writing on a DS that is in use we
		// need to do some basic buffering. Just to keep it safe and simple we'll have 10 buffers.
		VkDescriptorSet descriptorSet = m_descriptorSet[m_descriptorIndex];
		m_descriptorIndex = (m_descriptorIndex + 1) % s_descriptorBuffers;

		// Modify Descriptor set.
		SetDescriptorSet(m_pDevice->GetDevice(), 1, HDRSRV, &m_sampler, descriptorSet);
		m_pDynamicBufferRing->SetDescriptorSet(0, sizeof(ColorConversionConsts), descriptorSet);

		// Draw!
		m_colorConversion.Draw(cmd_buf, &cbTonemappingHandle, descriptorSet);

		SetPerfMarkerEnd(cmd_buf);
	}
}