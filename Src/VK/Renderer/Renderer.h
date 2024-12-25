#pragma once

#include "VK/Common/stdafx.h"

#include "VK/Base/GBuffer.h"
#include "VK/PostProcess/MagnifierPS.h"

// We are queuing ( backBufferCount + 0.5) frames, so we need to triple buffer the resource that get modified each frame.
static const int backBufferCount = 3;

//using namespace Engine_VK;

struct UIState;

// Renderer class is responsible rendering resource management and recording command buffers.
class Renderer
{
public:
	void OnCreate(Device* pDevice, SwapChain* pSwapChain, float fontSize);
	void OnDestroy();
	
	void OnCreateWindowSizeDependentResources( SwapChain* pSwapChain, uint32_t width, uint32_t height );
	void OnDestroyWindowSizeDependentResources();

	void OnUpdateDisplayDependentResources( SwapChain* pSwapChain, bool bUseMagnifier );
	void OnUpdateLocalDimmingChangedResources( SwapChain* pSwapChain );

	int LoadScene(GLTFCommon* pGLTFCommon, int Stage = 0);
	void UnloadScene();

	void AllocateShadowMaps(GLTFCommon* pGLTFCommon);

	const std::vector<TimeStamp>& GetTimingValues() { return m_timeStamps; }
	void OnRender(const UIState* pState, const Camera& cam, SwapChain* pSwapChain);

private:
	Device*							m_pDevice;

	uint32_t						m_width;
	uint32_t						m_height;

	VkRect2D						m_rectScissor;
	VkViewport						m_viewport;

	// Initialize helper classes.
	ResourceViewHeaps				m_resourceViewHeaps;
	UploadHeap						m_uploadHeap;
	DynamicBufferRing				m_constantBufferRing;
	StaticBufferPool				m_vidMemBufferPool;
	StaticBufferPool				m_sysMemBufferPool;
	CommandListRing					m_commandListRing;
	GPUTimestamps					m_gpuTimer;

	//gltf passes.
	GltfPbrPass*					m_GLTFPBR;
	GltfDepthPass*					m_GLTFDepth;
	GLTFTexturesAndBuffers*			m_pGLTFTexturesAndBuffers;

	// Effects.
	SkyDome							m_skyDome;
	DownSamplePS					m_downSample;
	MagnifierPS						m_magnifierPS;
	SkyDomeProc						m_skyDomeProc;
	ColorConversionPS				m_colorConversionPS;

	// GUI
	ImGUI							m_ImGUI;

	// GBuffer and Render passes
	GBuffer							m_gBuffer;
	GBufferRenderPass				m_renderPassFullGBufferWithClear;
	GBufferRenderPass				m_renderPassJustDepthAndHdr;
	GBufferRenderPass				m_renderPassFullGBuffer;

	// ShadowMaps
	VkRenderPass					m_renderPass_shadows;

	typedef struct
	{
		Texture						ShadowMap;
		uint32_t					ShadowIndex;
		uint32_t					ShadowResolution;
		uint32_t					LightIndex;
		VkImageView					ShadowDSV;
		VkFramebuffer				ShadowFrameBuffer;
	
	} SceneShadowInfo;

	std::vector<SceneShadowInfo>	m_shadowMapPool;
	std::vector<VkImageView>		m_shadowSRVPool;

	std::vector<TimeStamp>			m_timeStamps;
	AsyncPool						m_AsyncPool;
};
