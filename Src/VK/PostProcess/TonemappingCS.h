#pragma once

#include "PostProcCS.h"
#include "VK/Base/ResourceViewHeaps.h"

namespace Engine_VK
{
	class ToneMappingCS
	{
	public:
		void OnCreate(Device* pDevice, ResourceViewHeaps* pResourceViewHeaps, DynamicBufferRing* pDynamicBufferRing);
		void OnDestroy();

		void Draw(VkCommandBuffer cmd_buf, VkImageView HDRSRV, float exposure, int tonemapper, int width, int height);
				
	private:
		Device* m_pDevice;
		ResourceViewHeaps*		m_pResourceViewHeaps;

		PostProcCS				m_toneMapping;
		DynamicBufferRing*		m_pDynamicBufferRing = NULL;

		uint32_t				m_descriptorIndex;
		static const uint32_t	s_descriptorBuffers = 10;

		VkDescriptorSet			m_descriptorSet[s_descriptorBuffers];
		VkDescriptorSetLayout	m_descriptorSetLayout;

		struct TonemappingConsts
		{
			float				exposure;
			float				toneMapper;
		};
	};
}