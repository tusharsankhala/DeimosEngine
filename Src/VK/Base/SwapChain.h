#pragma once

#include "Device.h"
#include "FreeSyncHDR.h"

namespace Engine_VK
{
	class SwapChain
	{
	public:
		void			OnCreate( Device* pDevice, uint32_t numberBackBuffers, HWND hWnd );
		void			OnDestroy();

		void			OnCreateWindowSizeDependentResources( uint32_t width, uint32_t height, bool vSyncOn, 
																DisplayMode displayMode = DISPLAYMODE_SDR, PresentationMode fullScreenMode = PRESENTATIONMODE_WINDOWED, bool enableLocalDimming = true );
		void			OnDestroyWindowSizeDependentResources();
		void			SetFullScreen( bool fullscreen );

		bool			IsModeSupported( DisplayMode displayMode, PresentationMode fullScreenMode = PRESENTATIONMODE_WINDOWED, bool enableLocalDimming = true);
		void			EnumerateDisplayModes( std::vector<DisplayMode>* pModes, std::vector<const char*>* pNames = NULL, bool includeFreesyncHDR = false, PresentationMode fullscreenMode = PRESENTATIONMODE_WINDOWED, bool enableLocalDimming = true );
	
		void			GetSemaphores(VkSemaphore* pImageAvailableSemaphore, VkSemaphore* pRenderFinishedSemaphores, VkFence* pCmdBufExecutedFences);
		VkResult		Present();
		uint32_t		WaitForSwapChain();

		// getters.
		VkImage			GetCurrentBackBuffer();
		VkImageView		GetCurrentBackBufferRTV();
		VkSwapchainKHR	GetSwapChain()			const	{ return m_swapChain; }
		VkFormat		GetFormat()				const	{ return m_swapChainFormat.format; }
		VkRenderPass	GetRenderPass()			const	{ return m_render_pass_swap_chain; }
		DisplayMode		GetDisplayMode()		const	{ return m_displayMode; }
		VkFramebuffer	GetFrameBuffer(int i )	const	{ return m_framebuffers[i]; }
		VkFramebuffer	GetCurrentFrameBuffer()	const { return m_framebuffers[m_imageIndex]; }

	private:
		void			CreateRTV();
		void			DestroyRTV();
		void			CreateRenderPass();
		void			DestroyRenderPass();

		HWND						m_hWnd;
		Device						*m_pDevice;

		VkSwapchainKHR				m_swapChain;
		VkSurfaceFormatKHR			m_swapChainFormat;

		VkQueue						m_presentQueue;

		DisplayMode					m_displayMode				= DISPLAYMODE_SDR;
		VkRenderPass				m_render_pass_swap_chain	= VK_NULL_HANDLE;

		std::vector<VkImage>		m_images;
		std::vector<VkImageView>	m_imageViews;
		std::vector<VkFramebuffer>	m_framebuffers;

		std::vector<VkFence>		m_cmdBufExecutedFences;
		std::vector<VkSemaphore>	m_imageAvailableSemaphores;
		std::vector<VkSemaphore>	m_renderFinishedSemaphores;

		uint32_t					m_imageIndex				= 0;
		uint32_t					m_backbufferCount;
		uint32_t					m_semaphoreIndex, m_prevSemaphoreIndex;

		bool						m_bVSyncOn					= false;
	};
}
