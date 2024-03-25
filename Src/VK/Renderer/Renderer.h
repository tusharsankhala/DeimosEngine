#pragma once

#include "VK/Common/stdafx.h"

#include "VK/Base/GBuffer.h"
#include "VK/PostProcess/MagnifierPS.h"

using namespace Engine_VK;

// We are queuing ( backBufferCount + 0.5) frames, so we need to triple buffer the resource that get modified each frame.
static const int backBufferCount = 3;

// Renderer class is responsible rendering resource management and recording command buffers.
class Renderer
{
public:
	void OnCreate( Device *pDevice, SwapChain *pSwapChain, float fontSize );
	void OnDestroy();
	
	void OnCreateWindowSizeDependentResources( SwapChain* pSwapChain, uint32_t width, uint32_t height );
	void OnDestoryWindowSizeDependentResources();

	void OnUpdateDisplayDependentResources( SwapChain* pSwapChain, bool bUseMagnifier );
	void OnUpdateLocalDimmingChangedResources( SwapChain* pSwapChain );

private:
	Device*				m_pDevice;

	uint32_t			m_width;
	uint32_t			m_height;

	VkRect2D			m_rectScissor;
	VkViewport			m_viewport;

	// Initialize helper classes.
	ResourceViewHeaps	m_resourceViewHeaps;
	UploadHeap			m_uploadHeap;
	DynamicBufferRing	m_constantBufferRing;
	StaticBufferPool	m_vidMemBufferPool;
	StaticBufferPool	m_sysMemBufferPool;
	CommandListRing		m_commandListRing;
	GPUTimestamps		m_gpuTimer;

	// GBuffer and Render passes
	GBuffer				m_gBuffer;
	GBufferRenderPass	m_renderPassFullGBufferWithClear;
	GBufferRenderPass	m_renderPassJustDepthAndHdr;
	GBufferRenderPass	m_renderPassFullGBuffer;

	// ShadowMaps
	VkRenderPass		m_renderPass_shadows;
};
