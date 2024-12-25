#pragma once

#include "PostProcPS.h"

namespace Engine_VK
{
	#define DOWNSAMPLEPS_MAX_MIP_LEVELS 12

	class DownSamplePS
	{
	public:
		void		OnCreate(Device* pDevice, ResourceViewHeaps* pResourceViewHeaps, DynamicBufferRing* m_pConstantBufferRing,
					  StaticBufferPool* pStaticBufferPool, VkFormat outFormat);

		void		OnDestroy();

		void		OnCreateWindowSizeDependentResources(uint32_t width, uint32_t height, Texture* pInput, int mips);
		void		OnDestroyWindowSizeDependentResources();

		void		Draw(VkCommandBuffer cmd_buf);
		Texture*	GetTexture() { return &m_result; }

		void		Gui();

		struct cbDownscale
		{
			float invWidth, invHeight;
			int mipLevel;
		};

	private:
		Device*					m_pDevice;
		VkFormat				m_outFormat;

		Texture					m_result;

		struct Pass
		{
			VkImageView			RTV;			// dest
			VkImageView			m_SRV;			// src
			VkFramebuffer		frameBuffer;
			VkDescriptorSet		descriptorSet;
		};

		Pass					m_mip[DOWNSAMPLEPS_MAX_MIP_LEVELS];

		StaticBufferPool*		m_pStaticBufferPool;
		ResourceViewHeaps*		m_pResourceViewHeaps;
		DynamicBufferRing*		m_pConstantBufferRing;

		uint32_t				m_width;
		uint32_t				m_height;
		int						m_mipCount;

		VkDescriptorSetLayout	m_descriptorSetLayout;

		PostProcPS				m_downscale;

		VkRenderPass			m_in;

		VkSampler				m_sampler;

	};
}