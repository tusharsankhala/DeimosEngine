#pragma once

#include "VK/Base/Device.h"
#include "VK/Base/SwapChain.h"
#include "VK/Base/Texture.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Base/ShaderCompilerHelper.h"
#include "VK/Base/ResourceViewHeaps.h"

namespace Engine_VK
{
	typedef enum GBufferFlagBits
	{
		GBUFFER_NONE				= 0,
		GBUFFER_DEPTH				= 1,
		GBUFFER_FORWARD				= 2,
		GBUFFER_MOTION_VECTORS		= 4,
		GBUFFER_NORMAL_BUFFER		= 8,
		GBUFFER_DIFFUSE				= 16,
		GBUFFER_SPECULAR_ROUGHNESS	= 32,

	} GBufferFlagBits;

	typedef uint32_t GBufferFlags;

	class GBuffer;

	class GBufferRenderPass
	{
	public:
		void						OnCreate( GBuffer* pBuffer, GBufferFlags flags, bool bClear, const std::string& name );
		void						OnDestroy();
		void						OnCreateWindowSizeDependentResources( uint32_t width, uint32_t height );
		void						OnDestroyWindowSizeDependentResources();
		void						BeginPass( VkCommandBuffer commandList, VkRect2D renderArea );
		void						EndPass( VkCommandBuffer commandList );
		void						GetCompilerDefines( DefineList& defines );
		VkRenderPass				GetRenderPass() const { return m_renderPass; }
		VkFramebuffer				GetFramebuffer() const { return m_frameBuffer; }
		VkSampleCountFlagBits		GetSampleCount();

	private:
		Device						*m_pDevice;
		GBufferFlags				 m_flags;
		GBuffer						*m_pGBuffer;
		VkRenderPass				 m_renderPass;
		VkFramebuffer				 m_frameBuffer;
		std::vector<VkClearValue>	 m_clearValues;
	};

	class GBuffer
	{
	public:
		void						OnCreate( Device* pDevice, ResourceViewHeaps* pHeaps, const std::map<GBufferFlags, VkFormat>& formats, int sampleCount );
		void						OnDestroy();

		void						OnCreateWindowSizeDependentResources( SwapChain* pSwapChain, uint32_t width, uint32_t height );
		void						OnDestroyWindowSizeDependentResources();

		void						GetAttachmentList( GBufferFlags flags, std::vector<VkImageView>* pAttachments, std::vector<VkClearValue>* pClearValues );
		VkRenderPass				CreateRenderPass( GBufferFlags flags, bool bClear );

		void						GetCompilerDefines( DefineList& defines );

		VkSampleCountFlagBits		GetSampleCount() const { return m_sampleCount; }
		Device*						GetDevice() { return m_pDevice; }

		// Depth Buffer
		Texture						m_depthBuffer;
		VkImageView					m_depthBufferDSV;
		VkImageView					m_depthBufferSRV;
		
		// Diffuse.
		Texture						m_diffuse;
		VkImageView					m_diffuseSRV;

		// Specular.
		Texture						m_specularRoughness;
		VkImageView					m_specularRoughnessSRV;

		// Motion Vectors.
		Texture						m_motionVectors;
		VkImageView					m_motionVectorsSRV;

		// Normal Buffer.
		Texture						m_normalBuffer;
		VkImageView					m_normalBufferSRV;

		// HDR.
		Texture						m_hdr;
		VkImageView					m_hdrSRV;

	private:

		Device*								m_pDevice;
		
		VkSampleCountFlagBits				m_sampleCount;

		GBufferFlags						m_GBufferFlags;
		std::vector<VkClearValue>			m_clearValues;

		std::map<GBufferFlags, VkFormat>	m_formats;
	};
}
