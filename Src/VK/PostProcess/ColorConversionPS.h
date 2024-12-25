#pragma once

#include "PostProcPS.h"
#include "VK/Base/ResourceViewHeaps.h"

namespace Engine_VK
{
	class ColorConversionPS
	{
	public:
		void OnCreate(Device* pDevice, VkRenderPass renderPass, ResourceViewHeaps* pResourceViewHeaps, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pConstantBufferRing);
		void OnDestroy();

		void UpdatePipeline(VkRenderPass renderPass, DisplayMode displayMode);
		void Draw(VkCommandBuffer cmd_buf, VkImageView HDRSRV);
		
		
		struct cbDownscale
		{
			float invWidth, invHeight;
			int mipLevel;
		};

	private:
		Device*					m_pDevice;
		ResourceViewHeaps*		m_pResourceViewHeaps;

		PostProcPS				m_colorConversion;
		DynamicBufferRing*		m_pDynamicBufferRing = NULL;

		VkSampler				m_sampler;

		uint32_t				m_descriptorIndex;
		static const uint32_t	s_descriptorBuffers = 10;

		VkDescriptorSet			m_descriptorSet[s_descriptorBuffers];
		VkDescriptorSetLayout	m_descriptorSetLayout;

		struct ColorConversionConsts
		{
			math::Matrix4		m_contentToMonitorRecMatrix;
			DisplayMode			m_displayMode;
			float				m_displayMinLuminancePerNits;
			float				m_displayMaxLuminancePerNits;
		};

		ColorConversionConsts	m_colorConversionConsts;
	};
}