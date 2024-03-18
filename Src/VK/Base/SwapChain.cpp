#include "Common/stdafx.h"
#include "SwapChain.h"
#include "VK/Extensions/ExtFreeSyncHDR.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include <vulkan/vulkan_win32.h>

namespace Engine_VK
{
	//--------------------------------------------------------------------------------------
	//
	// OnCreate
	//
	//--------------------------------------------------------------------------------------
	void SwapChain::OnCreate(Device* pDevice, uint32_t numberBackBuffers, HWND hWnd)
	{
		VkResult res;

		m_hWnd					= hWnd;
		m_pDevice				= pDevice;
		m_backbufferCount		= numberBackBuffers;
		m_semaphoreIndex		= 0;
		m_prevSemaphoreCount	= 0;

		m_presentQueue			= pDevice->GetPresentQueue();

		// Init FSHDR.
		//fsHdrInit( pDevice->GetDevice(), pDevice->GetSurface(), pDevice->GetPhysicalDevice(), hWnd );

		// Set some safe format to start with.
		m_displayMode = DISPLAYMODE_SDR;
		VkSurfaceFormatKHR surfaceFormat;
		surfaceFormat.format		= VK_FORMAT_R8G8B8A8_UNORM;
		surfaceFormat.colorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		m_swapChainFormat = surfaceFormat;

		VkDevice device = m_pDevice->GetDevice();

		// Create fences and semaphore.
		//
		m_cmdBufExecutedFences.resize( m_backbufferCount );
		m_imageAvailableSemaphores.resize( m_backbufferCount);
		m_renderFinishedSemaphores.resize( m_backbufferCount );
		for( uint32_t i = 0; i < m_backbufferCount; ++i )
		{
			VkFenceCreateInfo fence_ci = {};
			fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			// The first call to WaitForSwapChain will wait on the fence 0 but use the frence 1 without resetting it
			// so any fence except 0 should be in the unsignalled state at he beginning.
			fence_ci.flags = i == 0 ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

			res = vkCreateFence( device, &fence_ci, NULL, &m_cmdBufExecutedFences[i] );
			assert( res == VK_SUCCESS );

			VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo = {};
			imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			res = vkCreateSemaphore( device, &imageAcquiredSemaphoreCreateInfo, NULL, &m_imageAvailableSemaphores[ i ] );
			assert( res == VK_SUCCESS );
			res = vkCreateSemaphore( device, &imageAcquiredSemaphoreCreateInfo, NULL, &m_renderFinishedSemaphores[ i ] );
			assert( res == VK_SUCCESS );
		}

		// If SDR the use a gamma corrected swapchain so that blending is correct.
		if( m_displayMode == DISPLAYMODE_SDR )
		{
			assert( m_swapChainFormat.format == VK_FORMAT_R8G8B8A8_UNORM );
			m_swapChainFormat.format = VK_FORMAT_R8G8B8A8_SRGB;
		}

		CreateRenderPass();
	}

	//--------------------------------------------------------------------------------------
	//
	// OnDestroy
	//
	//--------------------------------------------------------------------------------------
	void SwapChain::OnDestroy()
	{
		DestroyRenderPass();

		for( int i=0; i < m_cmdBufExecutedFences.size(); ++i )
		{
			vkDestroyFence( m_pDevice->GetDevice(), m_cmdBufExecutedFences[i], nullptr );
			vkDestroySemaphore( m_pDevice->GetDevice(), m_imageAvailableSemaphores[i], nullptr );
			vkDestroySemaphore(m_pDevice->GetDevice(), m_renderFinishedSemaphores[i], nullptr);
		}

	}

	//--------------------------------------------------------------------------------------
	//
	// EnumerateDisplayModes
	//
	//--------------------------------------------------------------------------------------
	void SwapChain::EnumerateDisplayModes( std::vector<DisplayMode> *pModes, std::vector<const char *> * pNames, bool includeFreesyncHDR,
										   PresentationMode fullscreenMode, bool enableLocalDimming)
	{
		fsHdrEnumerateDisplayModes();
	}

	bool SwapChain::IsModeSupported(DisplayMode displayMode, PresentationMode fullScreenMode, bool enableLocalDimming )
	{
		std::vector<DisplayMode> displayModesAvailable;
		EnumerateDisplayModes(&displayModesAvailable, nullptr, displayMode != DISPLAYMODE_SDR, fullScreenMode, enableLocalDimming );
		return std::find( displayModesAvailable.begin(), displayModesAvailable.end(), displayMode ) != displayModesAvailable.end();
	}


	void SwapChain::SetFullScreen( bool fullscreen )
	{
		if( ExtAreFSEExtensionsPresent() )
		{
			fsHdrSetFullscreenState( fullscreen, m_swapChain );
		}
	}

	void SwapChain::OnCreateWindowSizeDependentResources(uint32_t width, uint32_t height, bool vSyncOn,
														 DisplayMode displayMode, PresentationMode fullScreenMode, bool enableLocalDimming )
	{
		// Check whether the requested mode is supported and fallback to SDR if not supported.
		bool bIsModeSupported = IsModeSupported( displayMode, fullScreenMode, enableLocalDimming );
	}


	void SwapChain::OnDestroyWindowSizeDependentResources()
	{
		DestroyRenderPass();
	}

	void SwapChain::CreateRTV()
	{
		m_imageViews.resize( m_images.size() );
		for( uint32_t i = 0; i < m_images.size(); ++i )
		{
			VkImageViewCreateInfo color_image_view = {};
			color_image_view.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			color_image_view.pNext								= NULL;
			color_image_view.format								= m_swapChainFormat.format;
			color_image_view.components.r						= VK_COMPONENT_SWIZZLE_R;
			color_image_view.components.g						= VK_COMPONENT_SWIZZLE_G;
			color_image_view.components.b						= VK_COMPONENT_SWIZZLE_B;
			color_image_view.components.a						= VK_COMPONENT_SWIZZLE_A;
			color_image_view.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
			color_image_view.subresourceRange.baseMipLevel		= 0;
			color_image_view.subresourceRange.levelCount		= 1;
			color_image_view.subresourceRange.baseArrayLayer	= 0;
			color_image_view.subresourceRange.layerCount		= 1;
			color_image_view.viewType							= VK_IMAGE_VIEW_TYPE_2D;
			color_image_view.flags								= 0;
			color_image_view.image								= m_images[ i ];

			VkResult res = vkCreateImageView( m_pDevice->GetDevice(), &color_image_view, NULL, &m_imageViews[ i ] );
			assert( res == VK_SUCCESS );

			SetResourceName( m_pDevice->GetDevice(), VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t)m_imageViews[i], "SwapChain" );
		}
	}

	void SwapChain::DestroyRTV()
	{
		for( uint32_t i = 0; i < m_imageViews.size(); ++i )
		{
			vkDestroyImageView( m_pDevice->GetDevice(), m_imageViews[i], nullptr );
		}
	}


	void SwapChain::CreateRenderPass()
	{
		// Color RT.
		VkAttachmentDescription attachments[ 1 ];
		attachments[0].format			= m_swapChainFormat.format;
		attachments[0].samples			= VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments[0].flags			= 0;

		VkAttachmentReference color_reference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags					= 0;
		subpass.inputAttachmentCount	= 0;
		subpass.pInputAttachments		= NULL;
		subpass.colorAttachmentCount	= 1;
		subpass.pColorAttachments		= &color_reference;
		subpass.pResolveAttachments		= NULL;
		subpass.pDepthStencilAttachment	= NULL;
		subpass.preserveAttachmentCount	= 0;
		subpass.pPreserveAttachments	= NULL;

		VkSubpassDependency dep = {};
		dep.dependencyFlags		= 0;
		dep.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dep.dstStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.dstSubpass			= 0;
		dep.srcAccessMask		= 0;
		dep.srcStageMask		= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.srcSubpass			= VK_SUBPASS_EXTERNAL;

		VkRenderPassCreateInfo rp_info = {};
		rp_info.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext			= NULL;
		rp_info.attachmentCount = 1;
		rp_info.pAttachments	= attachments;
		rp_info.subpassCount	= 1;
		rp_info.pSubpasses		= &subpass;
		rp_info.dependencyCount	= 1;
		rp_info.pDependencies	= &dep;

		VkResult res = vkCreateRenderPass( m_pDevice->GetDevice(), &rp_info, NULL, &m_render_pass_swap_chain );
		assert( res == VK_SUCCESS );
	}

	void SwapChain::DestroyRenderPass()
	{
		if( m_render_pass_swap_chain != VK_NULL_HANDLE )
		{
			vkDestroyRenderPass( m_pDevice->GetDevice(), m_render_pass_swap_chain, nullptr );
			m_render_pass_swap_chain = VK_NULL_HANDLE;
		}
	}

	bool ExtAreFSEExtensionsPresent()
	{
		return s_isFSEDeviceExtensionsPresent;
	}
}