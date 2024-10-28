#pragma once

#include "VK/PostProcess/PostProcPS.h"
#include "VK/Base/Texture.h"

namespace Engine_VK
{
	// Forward Declerations.
	class DynamicBufferRing;
	class SwapChain;

	class MagnifierPS
	{
	public:
		//--------------------------------------------------------------------------------------------------------
		// DEFINITIONS
		//--------------------------------------------------------------------------------------------------------
		struct PassParameters
		{

		};

		uint32_t	uImageWidth;
		uint32_t	uImageHeight;
		int			iMousePos[2];					// in pixels, driven by ImGuiIO.MousePos.xy

		float		fBorderColorRGB[4];				// Linear RGBA

		float		fMagnificationAmount;			// [1-...]
		float		fMagnifierScreenRadius;			// [0-1]
		mutable int	iMagnifierOffset[2];			// in pixels

		//--------------------------------------------------------------------------------------------------------
		// DEFINITIONS
		//--------------------------------------------------------------------------------------------------------
	public:
		void OnCreate(Device*				pDevice,
					  ResourceViewHeaps*	pResourceViewHeaps,
					  DynamicBufferRing*	pDynamicBufferRing,
					  StaticBufferPool*		pStaticBufferPool,
					  VkFormat				outFormat,
					  bool					bOutputsToSwapchain = false);


		void OnDestroy();
		void OnCreateWindowSizeDependentResources(Texture* pTexture);
		void OnDestroyWindowSizeDependentResources();
		void BeginPass(VkCommandBuffer cmd, VkRect2D renerArea, SwapChain* pSwapChain = nullptr);
		void EndPass(VkCommandBuffer cmd);

		// Draws a manified region on top of the given image to onCreateWindowSizeDependentResources()
		// if @pSwapChain is provided, expects swapchain to be synced before this call.
		// Barriers are to be managed by the caller.
		void Draw(VkCommandBuffer cmd, const PassParameters params, SwapChain* pSwapChain = nullptr);

		inline Texture& GetPassOutput()
		{
			assert(!m_bOutputsToSwapchain);
			return m_texPassOutput;
		}

		inline VkImage GetPassOutputResource() const
		{
			assert(!m_bOutputsToSwapchain);
			return m_texPassOutput.Resource();
		}

		inline VkImageView GetPassOutputSRV() const
		{
			assert(!m_bOutputsToSwapchain);
			return m_srvOutput;
		}

		inline VkRenderPass GetPassRenderPass() const
		{
			assert(!m_bOutputsToSwapchain);
			return m_renderPass;
		}

		void UpdatePipelines(VkRenderPass renderPass);

	private:
		void CompileShaders(StaticBufferPool* pStaticBufferPool, VkFormat outFormat);
		void DestroyShaders();

		void InitializeDescriptorSets();
		void UpdateDescriptorSets(VkImageView imageViewSrc);
		void DestroyDescriptorSets();
		VkDescriptorBufferInfo SetConstantBufferData(const PassParameters params);
		static void KeepMagnifierOnScreen(const PassParameters& params);


		//--------------------------------------------------------------------------------------------------------
		// DATA
		//--------------------------------------------------------------------------------------------------------
	private:
		//
		// DEVICE & MEMORY
		//
		Device*					m_pDevice = nullptr;
		ResourceViewHeaps*		m_pResourceViewHeaps = nullptr;
		DynamicBufferRing*		m_pDynamicBufferRing = nullptr;

		//
		// DESCRIPTOR SETS & LAYOUTS
		//
		VkDescriptorSet			m_descriptorSet;
		VkDescriptorSetLayout	m_descriptorSetLayout;

		//
		// RENDER PASSES & RESOURCES VIEWS
		//
		VkSampler				m_samplerSrc;
		VkImageView				m_imageViewSrc;

		VkRenderPass			m_renderPass;
		VkFramebuffer			m_frameBuffer;
		VkImageView				m_rtvOutput;
		VkImageView				m_srvOutput;

		//
		// BUFFERSRENDER PASSES & RESOURCES VIEWS
		//
		// NONE


		//
		// TEXTURES
		//
		Texture					m_texPassOutput;

		//
		// SHADERS
		//
		PostProcPS				m_shaderMagnify;

		//
		// MISC
		//
		bool					m_bOutputsToSwapchain = false;
	};
}