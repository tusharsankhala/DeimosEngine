#include "Renderer.h"
#include "UI/UI.h"
//--------------------------------------------------------------------------------------
//
// OnCreate
//
//--------------------------------------------------------------------------------------
void Renderer::OnCreate( Device* pDevice, SwapChain* pSwapChain, float fontSize )
{
	m_pDevice = pDevice;

	// Initializie Helpers.

	// Create all the heaps for the resources views.
	const uint32_t	cbvDescriptorCount = 2000;
	const uint32_t	srvDescriptorCount = 8000;
	const uint32_t	uavDescriptorCount = 10;
	const uint32_t	sampleDescriptorCount = 20;
	m_resourceViewHeaps.OnCreate( pDevice, cbvDescriptorCount, srvDescriptorCount, uavDescriptorCount, sampleDescriptorCount );

	// Create a commandlist ring for the direct queue.
	uint32_t commandListPerBackBuffer = 8;
	m_commandListRing.OnCreate( pDevice, backBufferCount, commandListPerBackBuffer );

	// Create a 'dynamic' constant buffer.
	const uint32_t commandBuffersMemSize = 200 * 1024 * 1024;
	m_constantBufferRing.OnCreate( pDevice, backBufferCount, commandBuffersMemSize, (char*) "Uniforms" );

	// Create a 'static' pool for vertices and indices.
	const uint32_t staticGeometryMemSize = ( 1 * 128 ) * 1024 * 1024;
	m_vidMemBufferPool.OnCreate( pDevice, backBufferCount, commandBuffersMemSize, (char*)"Uniforms");

	// Create a 'static' pool for vertices and indices in the system memory.
	const uint32_t systemGeometryMemSize = 32 * 1024;
	m_sysMemBufferPool.OnCreate( pDevice, systemGeometryMemSize, false, "PostProcGame" );

	// Initialize the GPU time stamps module.
	m_gpuTimer.OnCreate( pDevice, backBufferCount );

	// Quick helper to upload resources, it has it's own commandList and uses suballocation.
	const uint32_t uploadHeapmemSize = 1000 * 1024 * 1024;

	// Initialize an upload heap ( uses suballocation for faster results )
	m_uploadHeap.OnCreate( pDevice, uploadHeapmemSize );

	// Create GBuffer and Render Passes.
	//
	{
		m_gBuffer.OnCreate( pDevice, &m_resourceViewHeaps,
			{
				{ GBUFFER_DEPTH, VK_FORMAT_D32_SFLOAT },
				{ GBUFFER_FORWARD, VK_FORMAT_R16G16B16A16_SFLOAT },
				{ GBUFFER_MOTION_VECTORS, VK_FORMAT_R16G16_SFLOAT },
			},
			1 );
		
		GBufferFlags fullGBuffer = GBUFFER_DEPTH | GBUFFER_FORWARD | GBUFFER_MOTION_VECTORS;
		bool bClear = true;
		m_renderPassFullGBufferWithClear.OnCreate( &m_gBuffer, fullGBuffer, bClear, "m_renderPassFullGBufferWithClear" );
		m_renderPassFullGBuffer.OnCreate( &m_gBuffer, fullGBuffer, !bClear, "m_renderPassFullGBuffer");
		m_renderPassJustDepthAndHdr.OnCreate( &m_gBuffer, GBUFFER_DEPTH | GBUFFER_FORWARD, !bClear, "m_renderPassJustDepthAndHdr" );
	}

	// Create render pass shadow, will clear contents
	{
		VkAttachmentDescription depthAttachments;
		AttachClearBeforeUse( VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &depthAttachments );
		m_renderPass_shadows = CreateRenderPassOptimal( m_pDevice->GetDevice(), 0, NULL, &depthAttachments );
	}

	// Initialize UI rendering resources.
	m_ImGUI.OnCreate(m_pDevice, pSwapChain->GetRenderPass(), &m_uploadHeap, &m_constantBufferRing, fontSize);

	// Make sure upload heap has finished uploading before continuing.
	m_uploadHeap.FlushAndFinish();
}

//--------------------------------------------------------------------------------------
//
// OnCreate
//
//--------------------------------------------------------------------------------------
void Renderer::OnDestroy()
{
	m_AsyncPool.Flush();

	m_ImGUI.OnDestroy();

	m_renderPassFullGBufferWithClear.OnDestroy();
	m_renderPassJustDepthAndHdr.OnDestroy();
	m_renderPassFullGBuffer.OnDestroy();
	m_gBuffer.OnDestroy();

	vkDestroyRenderPass(m_pDevice->GetDevice(), m_renderPass_shadows, nullptr);

	m_uploadHeap.OnDestroy();
	m_gpuTimer.OnDestroy();
	m_constantBufferRing.OnDestroy();
	m_resourceViewHeaps.OnDestroy();
	m_commandListRing.OnDestroy();
}

//--------------------------------------------------------------------------------------
//
// OnCreateWindowSizeDependentResources
//
//--------------------------------------------------------------------------------------

void Renderer::OnCreateWindowSizeDependentResources(SwapChain* pSwapChain, uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	// Set the viewport.
	m_viewport.x		= 0;
	m_viewport.y		= (float)height;
	m_viewport.width	= (float)width;
	m_viewport.height	= -(float)(height);
	m_viewport.minDepth = (float)0.0f;
	m_viewport.maxDepth = (float)1.0f;

	// Create scissor rectangle.
	//
	m_rectScissor.extent.width = width;
	m_rectScissor.extent.height = height;
	m_rectScissor.offset.x = 0;
	m_rectScissor.offset.y = 0;

	// Creating a GBuffer.
	// 
	m_gBuffer.OnCreateWindowSizeDependentResources(pSwapChain, width, height);

	// Create frame buffers for the GBuffer render passes.
	//
	m_renderPassFullGBufferWithClear.OnCreateWindowSizeDependentResources(width, height);
	m_renderPassJustDepthAndHdr.OnCreateWindowSizeDependentResources(width, height);
	m_renderPassFullGBuffer.OnCreateWindowSizeDependentResources(width, height);

}


//--------------------------------------------------------------------------------------
//
// OnDestoryWindowSizeDependentResources
//
//--------------------------------------------------------------------------------------
void Renderer::OnDestoryWindowSizeDependentResources()
{
	m_renderPassFullGBufferWithClear.OnDestroyWindowSizeDependentResources();
	m_renderPassJustDepthAndHdr.OnDestroyWindowSizeDependentResources();
	m_renderPassFullGBuffer.OnDestroyWindowSizeDependentResources();
	m_gBuffer.OnDestroyWindowSizeDependentResources();
}


//--------------------------------------------------------------------------------------
//
// OnUpdateDisplayDependentResources
//
//--------------------------------------------------------------------------------------
void Renderer::OnUpdateDisplayDependentResources(SwapChain* pSwapChain, bool bUseMagnifier)
{
	m_ImGUI.UpdatePipeline((pSwapChain->GetDisplayMode() == DISPLAYMODE_SDR) ? pSwapChain->GetRenderPass() : bUseMagnifier ? m_magnifierPS.GetPassRenderPass() : m_renderPassJustDepthAndHdr.GetRenderPass());
}

//--------------------------------------------------------------------------------------
//
// OnUpdateLocalDimmingChangedResources
//
//--------------------------------------------------------------------------------------
void Renderer::OnUpdateLocalDimmingChangedResources(SwapChain* pSwapChain)
{

}