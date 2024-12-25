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
void Renderer::OnDestroyWindowSizeDependentResources()
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
	m_colorConversionPS.UpdatePipeline(pSwapChain->GetRenderPass(), pSwapChain->GetDisplayMode());
}

//--------------------------------------------------------------------------------------
//
// LoadScene
//
//--------------------------------------------------------------------------------------
int Renderer::LoadScene(GLTFCommon* pGLTFCommon, int Stage)
{
	// Show loading progress.
	//
	ImGui::OpenPopup("Loading");
	if (ImGui::BeginPopupModal("Loading", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		float progress = (float)Stage / 12.0f;
		ImGui::ProgressBar(progress, ImVec2(0.f, 0.f), NULL);
		ImGui::EndPopup();
	}

	// use multi threading.
	AsyncPool* pAsyncPool = &m_AsyncPool;

	// Loading stages
	//
	if (Stage == 0)
	{
	}
	else if (Stage == 5)
	{
		Profile p("m_pGltfLoader->Load");
		
		m_pGLTFTexturesAndBuffers = new GLTFTexturesAndBuffers();
		m_pGLTFTexturesAndBuffers->OnCreate(m_pDevice, pGLTFCommon, &m_uploadHeap, &m_vidMemBufferPool, &m_constantBufferRing);
	}
	else if (Stage == 6)
	{
		Profile p("LoadTextures");

		// here we are loading onto the GPU all the texture and the inverse matrices
		// this data will be used to create the PBR and Depth passes.
		
		m_pGLTFTexturesAndBuffers->LoadTextures(pAsyncPool);	
	}
	else if (Stage == 7)
	{
		Profile p("m_GLTFDepth->OnCreate");

		// Create the glTF's textures, VBs and IBs, shaders and descriptors for this particular pass
		m_GLTFDepth = new GltfDepthPass();
		m_GLTFDepth->OnCreate(
			m_pDevice,
			m_renderPass_shadows,
			&m_uploadHeap,
			&m_resourceViewHeaps,
			&m_constantBufferRing,
			&m_vidMemBufferPool,
			m_pGLTFTexturesAndBuffers,
			pAsyncPool
		);

		m_vidMemBufferPool.UploadData(m_uploadHeap.GetCommandList());
		m_uploadHeap.FlushAndFinish();
	}
	else if (Stage == 8)
	{
		Profile p("m_GLTFPBR->OnCreate");

		// Same as the depth renderpass but for the PBR pass.
		m_GLTFPBR = new GltfPbrPass();
		m_GLTFPBR->OnCreate(
			m_pDevice,
			&m_uploadHeap,
			&m_resourceViewHeaps,
			&m_constantBufferRing,
			&m_vidMemBufferPool,
			m_pGLTFTexturesAndBuffers,
			&m_skyDome,
			false,			// use SSAO mask
			m_shadowSRVPool,
			&m_renderPassFullGBufferWithClear,
			pAsyncPool
		);

		m_vidMemBufferPool.UploadData(m_uploadHeap.GetCommandList());
		m_uploadHeap.FlushAndFinish();
	}
	else if (Stage == 9)
	{
		Profile p("m_GLTFBBox->OnCreate");

		// Just a boundng box pass that will draw boundingboxes instead of the geometry itself.
	}
	else if (Stage == 10)
	{
		Profile p("Flush");

		m_uploadHeap.FlushAndFinish();

		// once everything is uploaded we don't need the upload heaps anymore.
		m_vidMemBufferPool.FreeUploadHeap();

		// tell caller that we are done loading the map.
		return 0;
	}

	Stage++;
	return Stage;
}

//--------------------------------------------------------------------------------------
//
// UnloadScene
//
//--------------------------------------------------------------------------------------
void Renderer::UnloadScene()
{
	// wait for all the async loading operations to finish.
	m_AsyncPool.Flush();

	m_pDevice->GPUFlush();

	if (m_GLTFPBR)
	{
		m_GLTFPBR->OnDestroy();
		delete m_GLTFPBR;
		m_GLTFPBR = NULL;
	}

	if (m_GLTFDepth)
	{
		m_GLTFDepth->OnDestroy();
		delete m_GLTFDepth;
		m_GLTFDepth = NULL;
	}

	/*
	if (m_GLTFBBox)
	{
		m_GLTFBBox->OnDestroy();
		delete m_GLTFBBox;
		m_GLTFBBox = NULL;
	}
	*/

	if (m_pGLTFTexturesAndBuffers)
	{
		m_pGLTFTexturesAndBuffers->OnDestroy();
		delete m_pGLTFTexturesAndBuffers;
		m_pGLTFTexturesAndBuffers = NULL;
	}

	assert(m_shadowMapPool.size() == m_shadowSRVPool.size());
	while (!m_shadowMapPool.empty())
	{
		m_shadowMapPool.back().ShadowMap.OnDestroy();
		vkDestroyFramebuffer(m_pDevice->GetDevice(), m_shadowMapPool.back().ShadowFrameBuffer, nullptr);
		vkDestroyImageView(m_pDevice->GetDevice(), m_shadowSRVPool.back(), nullptr);
		vkDestroyImageView(m_pDevice->GetDevice(), m_shadowMapPool.back().ShadowDSV, nullptr);
		m_shadowSRVPool.pop_back();
		m_shadowMapPool.pop_back();
	}
}

void Renderer::AllocateShadowMaps(GLTFCommon* pGLTFCommon)
{
	// Gop through the lights and allocate shadow information.
	uint32_t NumShadows = 0;
	for( int i = 0; i < pGLTFCommon->m_lightInstances.size(); ++i)
	{
		const tfLight& lightData = pGLTFCommon->m_lights[pGLTFCommon->m_lightInstances[i].m_lightId];
		if (lightData.m_shadowResolution)
		{
			SceneShadowInfo shadowInfo;
			shadowInfo.ShadowResolution = lightData.m_shadowResolution;
			shadowInfo.ShadowIndex = NumShadows++;
			shadowInfo.LightIndex = i;
			m_shadowMapPool.push_back(shadowInfo);
		}
	}

	if (NumShadows > MaxShadowInstances)
	{
		Trace("Number of shadows has exceeded maximum suported. Please grow value in gltfCommon.h/perFrameStruct.h");
		throw;
	}

	// if we has shadow information, allocate all required maps and bindings.
	if (!m_shadowMapPool.empty())
	{
		std::vector<SceneShadowInfo>::iterator currentShadow = m_shadowMapPool.begin();
		for (uint32_t i = 0; currentShadow < m_shadowMapPool.end(); ++i, ++currentShadow)
		{
			currentShadow->ShadowMap.InitDepthStencil(m_pDevice, currentShadow->ShadowResolution, currentShadow->ShadowResolution,
				VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, "ShadowMap");
			currentShadow->ShadowMap.CreateDSV(&currentShadow->ShadowDSV);

			// Create render pass shadow, will clear contents.
			{
				VkAttachmentDescription depthAttachments;
				AttachClearBeforeUse(VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					&depthAttachments);

				// Create framebuffer
				VkImageView attachmentViews[1] = { currentShadow->ShadowDSV };
				VkFramebufferCreateInfo fb_info = { };
				fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				fb_info.pNext = NULL;
				fb_info.renderPass = m_renderPass_shadows;
				fb_info.attachmentCount = 1;
				fb_info.pAttachments = attachmentViews;
				fb_info.width = currentShadow->ShadowResolution;
				fb_info.height = currentShadow->ShadowResolution;
				fb_info.layers = 1;
				VkResult res = vkCreateFramebuffer(m_pDevice->GetDevice(), &fb_info, NULL, &currentShadow->ShadowFrameBuffer);
				assert(res == VK_SUCCESS);
			}

			VkImageView shadowSRV;
			currentShadow->ShadowMap.CreateSRV(&shadowSRV);
			m_shadowSRVPool.push_back(shadowSRV);
		}
	}
}

//--------------------------------------------------------------------------------------
//
// OnRender
//
//--------------------------------------------------------------------------------------
void Renderer::OnRender(const UIState* pState, const Camera& cam, SwapChain* pSwapChain)
{
	// Let our resource managers do some house keeping.
	m_constantBufferRing.OnBeginFrame();

	// command buffer calls.
	VkCommandBuffer cmdBuf1 = m_commandListRing.GetNewCommandList();

	{
		VkCommandBufferBeginInfo cmd_buf_info;
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext = NULL;
		cmd_buf_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		cmd_buf_info.pInheritanceInfo = NULL;
		VkResult res = vkBeginCommandBuffer(cmdBuf1, &cmd_buf_info);
		assert(res == VK_SUCCESS);
	}

	m_gpuTimer.OnBeginFrame(cmdBuf1, &m_timeStamps);

	// Sets the perFrame data.
	per_frame* pPerFrame = NULL;
	if (m_pGLTFTexturesAndBuffers)
	{
		// Fill as much as possible using the GLTF ( camera, lights, ...)
		pPerFrame = m_pGLTFTexturesAndBuffers->m_pGLTFCommon->SetPerFrameData(cam);

		// Set some lighting factors.
		pPerFrame->iblFactor = pState->IBLFactor;
		pPerFrame->emissiveFactor = pState->EmissiveFactor;
		pPerFrame->invScreenResolution[0] = 1.0f / ((float)m_width);
		pPerFrame->invScreenResolution[1] = 1.0f / ((float)m_height);

		pPerFrame->wireframeOptions.setX(pState->WireframeColor[0]);
		pPerFrame->wireframeOptions.setY(pState->WireframeColor[1]);
		pPerFrame->wireframeOptions.setZ(pState->WireframeColor[2]);
		pPerFrame->wireframeOptions.setW(pState->WireframeMode == UIState::WireframeMode::WIREFRAME_MODE_SOLID_COLOR ? 1.0f : 0.0f);
		pPerFrame->lodBias = 0.0f;
		m_pGLTFTexturesAndBuffers->SetPerFrameConstants();
		m_pGLTFTexturesAndBuffers->SetSkinningMatricesForSkeleton();
	}

	// Render all shadow maps.
	if (m_GLTFDepth && pPerFrame != NULL)
	{
		SetPerfMarkerBegin(cmdBuf1, "ShadowPass");

		VkClearValue depth_clear_values[1];
		depth_clear_values[0].depthStencil.depth	= 1.0f;
		depth_clear_values[0].depthStencil.stencil	= 0;

		VkRenderPassBeginInfo rp_begin;
		rp_begin.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin.pNext					= NULL;
		rp_begin.renderPass				= m_renderPass_shadows;
		rp_begin.renderArea.offset.x	= 0;
		rp_begin.renderArea.offset.y	= 0;
		rp_begin.clearValueCount		= 1;
		rp_begin.pClearValues			= depth_clear_values;

		std::vector<SceneShadowInfo>::iterator shadowMap = m_shadowMapPool.begin();
		while (shadowMap < m_shadowMapPool.end())
		{
			// Clear shadow map.
			rp_begin.framebuffer				= shadowMap->ShadowFrameBuffer;
			rp_begin.renderArea.extent.width	= shadowMap->ShadowResolution;
			rp_begin.renderArea.extent.height	= shadowMap->ShadowResolution;
			vkCmdBeginRenderPass(cmdBuf1, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

			// Render to shadow map.
			SetViewportAndScissor(cmdBuf1, 0, 0, shadowMap->ShadowResolution, shadowMap->ShadowResolution);

			// Set per frame constant buffer values.
			GltfDepthPass::per_frame* cbPerFrame = m_GLTFDepth->SetPerFrameConstants();
			cbPerFrame->m_viewProj = pPerFrame->lights[shadowMap->LightIndex].m_lightViewProj;

			m_GLTFDepth->Draw(cmdBuf1);

			m_gpuTimer.GetTimeStamp(cmdBuf1, "Shadow Map Render");
			vkCmdEndRenderPass(cmdBuf1);
			++shadowMap;
		}

		SetPerfMarkerEnd(cmdBuf1);
	}

	// Render Scene to the GBuffer ------------------------------------------------
	SetPerfMarkerBegin(cmdBuf1, "Color Pass");

	VkRect2D renderArea = { 0, 0, m_width, m_height };

	if (pPerFrame != NULL && m_GLTFPBR)
	{
		const bool bWireframe = pState->WireframeMode != UIState::WireframeMode::WIREFRAME_MODE_OFF;

		std::vector<GltfPbrPass::BatchList> opaque, transparent;
		m_GLTFPBR->BuildBatchLists(&opaque, &transparent, bWireframe);

		// Render Opaque
		{
			m_renderPassFullGBufferWithClear.BeginPass(cmdBuf1, renderArea);

			m_GLTFPBR->DrawBatchList(cmdBuf1, &opaque, bWireframe);
			m_gpuTimer.GetTimeStamp(cmdBuf1, "PBR Opaquee");

			m_renderPassFullGBufferWithClear.EndPass(cmdBuf1);
		}

		// Render skydome
		{
			m_renderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);

			if (pState->SelectedSkydomeTypeIndex == 1)
			{
				math::Matrix4 clipToView = math::inverse(pPerFrame->m_cameraCurrViewProj);
				m_skyDome.Draw(cmdBuf1, clipToView);

				m_gpuTimer.GetTimeStamp(cmdBuf1, "Skydome cube");
			}
			else if (pState->SelectedSkydomeTypeIndex == 0)
			{
				SkyDomeProc::Constants skyDomeConstants;
				skyDomeConstants.invViewProj		= math::inverse(pPerFrame->m_cameraCurrViewProj);
				skyDomeConstants.vSunDirection		= math::Vector4(1.0f, 0.05f, 0.0f, 0.0f);
				skyDomeConstants.turbidity			= 10.0f;
				skyDomeConstants.rayleigh			= 2.0f;
				skyDomeConstants.mieCoefficient		= 0.005f;
				skyDomeConstants.mieDirectionalG	= 0.8f;
				skyDomeConstants.luminance			= 1.0f;
				m_skyDomeProc.Draw(cmdBuf1, skyDomeConstants);

				m_gpuTimer.GetTimeStamp(cmdBuf1, "Skydome Proc");
			}

			m_renderPassJustDepthAndHdr.EndPass(cmdBuf1);
		}

		// Draw transparent geometry.
		{
			m_renderPassFullGBuffer.BeginPass(cmdBuf1, renderArea);

			std::sort(transparent.begin(), transparent.end());
			m_GLTFPBR->DrawBatchList(cmdBuf1, &transparent, bWireframe);
			m_gpuTimer.GetTimeStamp(cmdBuf1, "PBR Transparent");

			m_renderPassFullGBuffer.EndPass(cmdBuf1);
		}

		// draw object's bounding boxes.
		{
			m_renderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);
			/*
			if (m_GLTFBBox)
			{
				if(pState)
			}
			*/

			// draw lights's frustum.
			if (pState->bDrawLightFrustum && pPerFrame != NULL)
			{
				SetPerfMarkerBegin(cmdBuf1, "Light Frustums");

				math::Vector4 vCenter	= math::Vector4(0.0f, 0.0f, 0.5f, 0.0f);
				math::Vector4 vRadius	= math::Vector4(1.0f, 1.0f, 0.5f, 0.0f);
				math::Vector4 vColor	= math::Vector4(1.0f, 1.0f, 1.0f, 1.0f);

				for (uint32_t i = 0; i < pPerFrame->lightCount; ++i)
				{
					math::Matrix4 spotLightMatrix = math::inverse(pPerFrame->lights[i].m_lightViewProj);
					math::Matrix4 worldMatrix = pPerFrame->m_cameraCurrViewProj * spotLightMatrix;
					//m_wirefram
				}

				m_gpuTimer.GetTimeStamp(cmdBuf1, "Light's Frustum");

				SetPerfMarkerEnd(cmdBuf1);
			}

			m_renderPassJustDepthAndHdr.EndPass(cmdBuf1);
		}
	}
	else
	{
		m_renderPassFullGBufferWithClear.BeginPass(cmdBuf1, renderArea);
		m_renderPassFullGBufferWithClear.EndPass(cmdBuf1);
		m_renderPassJustDepthAndHdr.BeginPass(cmdBuf1, renderArea);
		m_renderPassJustDepthAndHdr.EndPass(cmdBuf1);
	}

	VkImageMemoryBarrier barrier[1] = {};
	barrier[0].sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier[0].pNext							= NULL;
	barrier[0].srcAccessMask					= VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier[0].dstAccessMask					= VK_ACCESS_SHADER_READ_BIT;
	barrier[0].oldLayout						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier[0].newLayout						= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier[0].srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier[0].dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier[0].subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier[0].subresourceRange.baseMipLevel	= 0;
	barrier[0].subresourceRange.levelCount		= 1;
	barrier[0].subresourceRange.baseArrayLayer	= 0;
	barrier[0].subresourceRange.layerCount		= 1;
	barrier[0].image							= m_gBuffer.m_hdr.Resource();
	vkCmdPipelineBarrier(cmdBuf1, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						 0, 0, NULL, 0, NULL, 1, barrier);

	SetPerfMarkerEnd(cmdBuf1);

	// Post Proc ---------------------------------------------------------------

	// Bloom, takes HDR as input and applies bloom to it.
	{
		SetPerfMarkerBegin( cmdBuf1, "PostProcess");

		// Downsample pass.
		m_downSample.Draw(cmdBuf1);
		m_gpuTimer.GetTimeStamp(cmdBuf1, "Downsample");

		// Bloom pass ( needs the downsampled data)
		m_gpuTimer.GetTimeStamp(cmdBuf1, "Bloom");

		SetPerfMarkerEnd(cmdBuf1);
	}

	// Start tracking input/output resources at this point to handle HDR and SDR rener paths.
	VkImage		ImgCurrentInput	= m_gBuffer.m_hdr.Resource();
	VkImageView SRVCurrentInput = m_gBuffer.m_hdrSRV;

	// If using FreeSync HDR, we need to do these in order: Tonemapping -> GUI -> Color Conversion
	const bool bHDR = pSwapChain->GetDisplayMode() != DISPLAYMODE_SDR;

	// submit command buffer.
	{
		VkResult res = vkEndCommandBuffer(cmdBuf1);
		assert(res == VK_SUCCESS);

		VkSubmitInfo submit_info;
		submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext					= NULL;
		submit_info.waitSemaphoreCount		= 0;
		submit_info.pWaitSemaphores			= NULL;
		submit_info.commandBufferCount		= 1;
		submit_info.pCommandBuffers			= &cmdBuf1;
		submit_info.signalSemaphoreCount	= 0;
		submit_info.pSignalSemaphores		= NULL;
		res = vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
		assert(res == VK_SUCCESS);
		
	}

	// Wait for swapchain ( we are going to render to it) ------------------------------
	int imageIndex = pSwapChain->WaitForSwapChain();

	// Keep tracking input/output resource views.
	ImgCurrentInput				= m_gBuffer.m_hdr.Resource();
	SRVCurrentInput = m_gBuffer.m_hdrSRV;

	m_commandListRing.OnBeginFrame();

	VkCommandBuffer cmdBuf2		= m_commandListRing.GetNewCommandList();

	{
		VkCommandBufferBeginInfo cmd_buf_info;
		cmd_buf_info.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buf_info.pNext				= NULL;
		cmd_buf_info.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		cmd_buf_info.pInheritanceInfo	= NULL;
		VkResult res = vkBeginCommandBuffer(cmdBuf2, &cmd_buf_info);
		assert(res == VK_SUCCESS);
	}

	SetPerfMarkerBegin(cmdBuf2, "Swapchain RenderPass");

	vkCmdSetScissor(cmdBuf2, 0, 1, &m_rectScissor);
	vkCmdSetViewport(cmdBuf2, 0, 1, &m_viewport);

	if (bHDR)
	{
		//m_colorConversionPS.Draw(cmdBuf2, SRVCurrentInput);
		m_gpuTimer.GetTimeStamp(cmdBuf2, "Color Conversion");
	}
	// For SDR pipeline, we apply the tonemapping and then draw the GUI and skip
	else
	{
		// Tonemapping --------------------------------------------------------------
		{
			//m_tonemappin
		}

		// Render HUD --------------------------------------------------------------
		{
			m_ImGUI.Draw(cmdBuf2);
			m_gpuTimer.GetTimeStamp(cmdBuf2, "ImGUI Rendering");
		}
	}

	SetPerfMarkerEnd(cmdBuf2);

	m_gpuTimer.OnEndFrame();

	vkCmdEndRenderPass(cmdBuf2);

	// Close and Submit the command list ----------------------------------------------
	{
		VkResult res = vkEndCommandBuffer(cmdBuf2);
		assert(res == VK_SUCCESS);

		VkSemaphore	imageAvailabeSemaphore;
		VkSemaphore	renderFinishedSemaphore;
		VkFence		cmdBufExecuteFences;
		pSwapChain->GetSemaphores(&imageAvailabeSemaphore, &renderFinishedSemaphore, &cmdBufExecuteFences);

		VkPipelineStageFlags submitWaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit_info2;
		submit_info2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info2.pNext = NULL;
		submit_info2.waitSemaphoreCount = 1;
		submit_info2.pWaitSemaphores = &imageAvailabeSemaphore;
		submit_info2.pWaitDstStageMask = &submitWaitStage;
		submit_info2.commandBufferCount = 1;
		submit_info2.pCommandBuffers = &cmdBuf2;
		submit_info2.signalSemaphoreCount = 1;
		submit_info2.pSignalSemaphores = &renderFinishedSemaphore;

		res = vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submit_info2, cmdBufExecuteFences);
		assert(res == VK_SUCCESS);
	}
}