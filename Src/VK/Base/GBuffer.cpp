#include "Common/stdafx.h"
#include "VK/Base/GBuffer.h"
#include "VK/Base/Helper.h"
#include <assert.h>

namespace Engine_VK
{
	void GBufferRenderPass::OnCreate( GBuffer* pGBuffer, GBufferFlags flags, bool bClear, const std::string& name )
	{
		m_flags		= flags;
		m_pGBuffer	= pGBuffer;
		m_pDevice	= pGBuffer->GetDevice();

		m_renderPass = pGBuffer->CreateRenderPass( flags, bClear );
		SetResourceName( m_pDevice->GetDevice(), VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)m_renderPass, name.c_str() );
	}

	void GBufferRenderPass::OnDestroy()
	{
		vkDestroyRenderPass( m_pGBuffer->GetDevice()->GetDevice(), m_renderPass, nullptr );
	}

	void GBufferRenderPass::OnCreateWindowSizeDependentResources( uint32_t width, uint32_t height )
	{
		std::vector<VkImageView> attachments;
		m_pGBuffer->GetAttachmentList( m_flags, &attachments, &m_clearValues );
		m_frameBuffer = CreateFrameBuffer( m_pGBuffer->GetDevice()->GetDevice(), m_renderPass, &attachments, width, height );
	}

	void GBufferRenderPass::OnDestroyWindowSizeDependentResources()
	{
		vkDestroyFramebuffer( m_pGBuffer->GetDevice()->GetDevice(), m_frameBuffer, nullptr );
	}

	void GBufferRenderPass::BeginPass(VkCommandBuffer commandList, VkRect2D renderArea)
	{
		VkRenderPassBeginInfo rp_begin;
		rp_begin.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin.pNext			= NULL;
		rp_begin.renderPass		= m_renderPass;
		rp_begin.framebuffer	= m_frameBuffer;
		rp_begin.renderArea		= renderArea;
		rp_begin.pClearValues	= m_clearValues.data();
		rp_begin.clearValueCount = (uint32_t)m_clearValues.size();
		vkCmdBeginRenderPass( commandList, &rp_begin, VK_SUBPASS_CONTENTS_INLINE );

		SetViewportAndScissor( commandList, renderArea.offset.x, renderArea.offset.y, renderArea.extent.width, renderArea.extent.height );
	}

	void GBufferRenderPass::EndPass(VkCommandBuffer commandList)
	{
		vkCmdEndRenderPass( commandList );
	}

	void GBufferRenderPass::GetCompilerDefines(DefineList& defines)
	{
		int rtIndex = 0;

		// GDR ( Forward Pass )
		//
		if( m_flags & GBUFFER_FORWARD )
		{
			defines["HAS_FORWARD_RT"] = std::to_string( rtIndex++ );
		}

		// Motion Vectors
		//
		if ( m_flags & GBUFFER_MOTION_VECTORS )
		{
			defines["HAS_MOTION_VECTORS"]		= std::to_string(1);
			defines["HAS_MOTION_VECTORS_RT"]	= std::to_string( rtIndex++ );
		}

		// Normal Buffer
		// 
		if ( m_flags & GBUFFER_NORMAL_BUFFER )
		{
			defines["HAS_NORMAL_RT"] = std::to_string( rtIndex++ );
		}

		// Diffuse.
		//
		if ( m_flags & GBUFFER_DIFFUSE )
		{
			defines["HAS_DIFFUSE_RT"] = std::to_string( rtIndex++ );
		}

		// Specular Roughness
		//
		if ( m_flags & GBUFFER_SPECULAR_ROUGHNESS )
		{
			defines["HAS_SPECULAR_ROUGHNESS_RT"] = std::to_string(rtIndex++);
		}

	}

	VkSampleCountFlagBits GBufferRenderPass::GetSampleCount()
	{
		return m_pGBuffer->GetSampleCount();
	}

	//-------------------------------------------------------------------------------------------------------

	void GBuffer::OnCreate( Device* pDevice, ResourceViewHeaps* pHeaps, const std::map<GBufferFlags, VkFormat>& formats, int sampleCount )
	{
		m_GBufferFlags = GBUFFER_NONE;
		for( auto a : formats )
			m_GBufferFlags = m_GBufferFlags | a.first;

		m_pDevice		= pDevice;
		m_sampleCount	= (VkSampleCountFlagBits)sampleCount;
		m_formats		= formats;
	}

	void GBuffer::OnDestroy()
	{
	}

	VkRenderPass GBuffer::CreateRenderPass( GBufferFlags flags, bool bClear )
	{
		VkAttachmentDescription depthAttachment;
		VkAttachmentDescription colorAttachments[10];
		uint32_t colorAttachmentCount = 0;

		auto addAttachment = bClear ? AttachClearBeforeUse : AttachBlending;
		VkImageLayout previousColor = bClear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkImageLayout previousDepth = bClear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		if( flags & GBUFFER_FORWARD )
		{
			addAttachment( m_formats[ GBUFFER_FORWARD], m_sampleCount, previousColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,&colorAttachments[ colorAttachmentCount++ ] );

			// Assert if the RT is not present in the GBuffer.
			assert( m_GBufferFlags & GBUFFER_FORWARD );
		}

		if ( flags & GBUFFER_MOTION_VECTORS )
		{
			addAttachment( m_formats[ GBUFFER_MOTION_VECTORS ], m_sampleCount, previousColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorAttachments[colorAttachmentCount++]);

			// Assert if the RT is not present in the GBuffer.
			assert( m_GBufferFlags & GBUFFER_MOTION_VECTORS );
		}

		if ( flags & GBUFFER_NORMAL_BUFFER )
		{
			addAttachment( m_formats[ GBUFFER_NORMAL_BUFFER ], m_sampleCount, previousColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorAttachments[colorAttachmentCount++]);

			// Assert if the RT is not present in the GBuffer.
			assert( m_GBufferFlags & GBUFFER_NORMAL_BUFFER );
		}

		if ( flags & GBUFFER_DIFFUSE )
		{
			addAttachment( m_formats[ GBUFFER_DIFFUSE ], m_sampleCount, previousColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorAttachments[colorAttachmentCount++]);

			// Assert if the RT is not present in the GBuffer.
			assert( m_GBufferFlags & GBUFFER_DIFFUSE );
		}

		if ( flags & GBUFFER_SPECULAR_ROUGHNESS )
		{
			addAttachment(m_formats[ GBUFFER_SPECULAR_ROUGHNESS ], m_sampleCount, previousColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorAttachments[colorAttachmentCount++]);

			// Assert if the RT is not present in the GBuffer.
			assert( m_GBufferFlags & GBUFFER_SPECULAR_ROUGHNESS );
		}

		if ( flags & GBUFFER_DEPTH )
		{
			addAttachment(m_formats[GBUFFER_DEPTH], m_sampleCount, previousColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &colorAttachments[colorAttachmentCount++]);

			// Assert if the RT is not present in the GBuffer.
			assert( m_GBufferFlags & GBUFFER_DEPTH );
		}

		return CreateRenderPassOptimal( m_pDevice->GetDevice(), colorAttachmentCount, colorAttachments, &depthAttachment );
	}

	void GBuffer::GetAttachmentList( GBufferFlags flags, std::vector<VkImageView>* pAttachments, std::vector<VkClearValue>* pClearValues )
	{
		pAttachments->clear();

		// Create Texture + RTV, to hold the resolved scene.
		//
		if( flags & GBUFFER_FORWARD )
		{
			pAttachments->push_back( m_hdrSRV );

			if( pClearValues )
			{
				VkClearValue cv;
				cv.color = { 0.0f, 0.0f, 0.00f, 0.0f };
				pClearValues->push_back( cv );
			}
		}

		// Motion Vectors.
		//
		if ( flags & GBUFFER_MOTION_VECTORS )
		{
			pAttachments->push_back( m_motionVectorsSRV );

			if ( pClearValues )
			{
				VkClearValue cv;
				cv.color = { 0.0f, 0.0f, 0.00f, 0.0f };
				pClearValues->push_back(cv);
			}
		}

		// Normal Buffers.
		//
		if ( flags & GBUFFER_NORMAL_BUFFER )
		{
			pAttachments->push_back( m_normalBufferSRV );

			if (pClearValues)
			{
				VkClearValue cv;
				cv.color = { 0.0f, 0.0f, 0.00f, 0.0f };
				pClearValues->push_back(cv);
			}
		}

		// Diffuse.
		//
		if ( flags & GBUFFER_DIFFUSE )
		{
			pAttachments->push_back( m_diffuseSRV );

			if ( pClearValues )
			{
				VkClearValue cv;
				cv.color = { 0.0f, 0.0f, 0.00f, 0.0f };
				pClearValues->push_back(cv);
			}
		}

		// Specular Roughness.
		//
		if ( flags & GBUFFER_SPECULAR_ROUGHNESS )
		{
			pAttachments->push_back( m_specularRoughnessSRV );

			if (pClearValues)
			{
				VkClearValue cv;
				cv.color = { 0.0f, 0.0f, 0.00f, 0.0f };
				pClearValues->push_back(cv);
			}
		}

		// Create Depth buffer.
		//
		if ( flags & GBUFFER_DEPTH )
		{
			pAttachments->push_back( m_depthBufferDSV );

			if ( pClearValues )
			{
				VkClearValue cv;
				cv.depthStencil = { 1.0f, 0 };
				pClearValues->push_back(cv);
			}
		}
	}

	void GBuffer::OnCreateWindowSizeDependentResources( SwapChain* pSwapChain, uint32_t width, uint32_t height )
	{
		// Create texture + RTV, to hold the resolved scene.
		//
		if ( m_GBufferFlags & GBUFFER_FORWARD )
		{
			m_hdr.InitRenderTarget( m_pDevice, width, height, m_formats[ GBUFFER_FORWARD ], m_sampleCount,
				(VkImageUsageFlags)( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT ), false, "m_HDR" );

			m_hdr.CreateSRV( &m_hdrSRV );
		}

		// Motion Vectors.
		//
		if ( m_GBufferFlags & GBUFFER_MOTION_VECTORS )
		{
			m_motionVectors.InitRenderTarget( m_pDevice, width, height, m_formats[ GBUFFER_MOTION_VECTORS ], m_sampleCount,
				(VkImageUsageFlags)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT), false, "m_MotionVector" );

			m_motionVectors.CreateSRV( &m_motionVectorsSRV );
		}

		// Normal Buffer.
		//
		if ( m_GBufferFlags & GBUFFER_NORMAL_BUFFER )
		{
			m_normalBuffer.InitRenderTarget( m_pDevice, width, height, m_formats[ GBUFFER_NORMAL_BUFFER ], m_sampleCount,
				(VkImageUsageFlags)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT), false, "m_NormalBuffer" );

			m_normalBuffer.CreateSRV( &m_normalBufferSRV );
		}

		// Diffuse.
		//
		if ( m_GBufferFlags & GBUFFER_DIFFUSE )
		{
			m_diffuse.InitRenderTarget( m_pDevice, width, height, m_formats[ GBUFFER_DIFFUSE ], m_sampleCount,
				(VkImageUsageFlags)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT), false, "m_Diffuse" );

			m_diffuse.CreateSRV( &m_diffuseSRV );
		}

		// Specular Roughness.
		//
		if ( m_GBufferFlags & GBUFFER_SPECULAR_ROUGHNESS )
		{
			m_specularRoughness.InitRenderTarget( m_pDevice, width, height, m_formats[ GBUFFER_SPECULAR_ROUGHNESS ], m_sampleCount,
				(VkImageUsageFlags)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT), false, "m_SpecularRoughness" );

			m_specularRoughness.CreateSRV( &m_specularRoughnessSRV );
		}

		// Create Depth Buffer.
		//
		if ( m_GBufferFlags & GBUFFER_DEPTH )
		{
			m_depthBuffer.InitDepthStencil( m_pDevice, width, height, m_formats[ GBUFFER_DEPTH ], m_sampleCount, "DepthBuffer" );
			m_depthBuffer.CreateDSV( &m_depthBufferDSV );
			m_depthBuffer.CreateRTV( &m_depthBufferSRV );
		}
	}

	void GBuffer::OnDestroyWindowSizeDependentResources()
	{
		if( m_GBufferFlags & GBUFFER_SPECULAR_ROUGHNESS )
		{
			vkDestroyImageView( m_pDevice->GetDevice(), m_specularRoughnessSRV, nullptr );
			m_specularRoughness.OnDestroy();
		}

		if ( m_GBufferFlags & GBUFFER_DIFFUSE )
		{
			vkDestroyImageView( m_pDevice->GetDevice(), m_diffuseSRV, nullptr);
			m_diffuse.OnDestroy();
		}

		if ( m_GBufferFlags & GBUFFER_NORMAL_BUFFER )
		{
			vkDestroyImageView(m_pDevice->GetDevice(), m_normalBufferSRV, nullptr);
			m_normalBuffer.OnDestroy();
		}

		if ( m_GBufferFlags & GBUFFER_MOTION_VECTORS )
		{
			vkDestroyImageView( m_pDevice->GetDevice(), m_motionVectorsSRV, nullptr );
			m_motionVectors.OnDestroy();
		}

		if ( m_GBufferFlags & GBUFFER_FORWARD )
		{
			vkDestroyImageView( m_pDevice->GetDevice(), m_hdrSRV, nullptr);
			m_hdr.OnDestroy();
		}

		if ( m_GBufferFlags & GBUFFER_DEPTH )
		{
			vkDestroyImageView( m_pDevice->GetDevice(), m_depthBufferDSV, nullptr );
			vkDestroyImageView( m_pDevice->GetDevice(), m_depthBufferDSV, nullptr );
			m_depthBuffer.OnDestroy();
		}
	}

	void GBuffer::GetCompilerDefines(DefineList& defines)
	{
	}
}