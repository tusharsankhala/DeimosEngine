#pragma once

#include "PostProcPS.h"
#include "VK/Base/ResourceViewHeaps.h"

namespace Engine_VK
{
	class ToneMapping
	{
	public:
		void OnCreate(Device* pDevice, VkRenderPass renerPass, ResourceViewHeaps* pResourceViewHeaps, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing, uint32_t srvTableSize = 1, const char* shaderSource = "Tonemapping.glsl");
		void OnDestroy();

		void UpdatePipeline(VkRenderPass renderPass);
		void Draw(VkCommandBuffer cmd_buf, VkImageView HDRSRV, float exposure, int tonemapper, int width, int height);

	private:

		Device* m_pDevice;
		ResourceViewHeaps*		m_pResourceViewHeaps;

		PostProcPS				m_toneMapping;
		DynamicBufferRing*		m_pDynamicBufferRing = NULL;

		VkSampler				m_sampler;

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