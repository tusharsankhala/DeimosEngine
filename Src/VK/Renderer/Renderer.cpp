#include "Renderer.h"

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
}