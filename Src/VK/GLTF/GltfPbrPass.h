#pragma once

#include "GLTFTexturesAndBuffers.h"
#include "VK/PostProcess/SkyDome.h"
#include "VK/Base/GBuffer.h"
#include "Common/GLTF/GltfPbrMaterial.h"

namespace Engine_VK
{
	// Material, primitive and mesh structs specific for the PBR pass
	// (you might want to compare these structs with the ones used for the depth pass in GltfDepthPass.h)
	struct PBRMaterial
	{
		int						m_textureCount = 0;
		VkDescriptorSet			m_texturesDescriptorSet				= VK_NULL_HANDLE;
		VkDescriptorSetLayout	m_texturesDescriptorSetLayout		= VK_NULL_HANDLE;

		PBRMaterialParameters	m_pbrMaterialParameters;
	};

	struct PBRPrimitives
	{
		Geometry 					m_geometry;
		
		PBRMaterial*				m_pMaterial						= NULL;

		VkPipeline					m_pipeline						= VK_NULL_HANDLE;
		VkPipeline					m_pipelineWireframe				= VK_NULL_HANDLE;
		VkPipelineLayout			m_pipelineLayout				= VK_NULL_HANDLE;

		VkDescriptorSet				m_uniformsDescriptorSet			= VK_NULL_HANDLE;
		VkDescriptorSetLayout		m_uniformsDescriptorSetLayout	= VK_NULL_HANDLE;

		void DrawPrimitive(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo perSceneDesc, VkDescriptorBufferInfo perObjectDesc, VkDescriptorBufferInfo* pPerSkeleton, bool bWireframe);
	};

	struct PBRMesh
	{
		std::vector<PBRPrimitives>	m_pPrimitives;
	};

	class GltfPbrPass
	{
	public:
		struct per_object
		{
			math::Matrix4			m_currentWorld;
			math::Matrix4			m_previousWorld;

			PBRMaterialParametersConstantBuffer m_pbrParams;
		};

		struct BatchList
		{
			float					m_depth;
			PBRPrimitives*			m_pPrimitive;
			VkDescriptorBufferInfo	m_perFrameDesc;
			VkDescriptorBufferInfo	m_perObjectDesc;
			VkDescriptorBufferInfo*	m_pPerSkeleton;
			operator float() { return -m_depth; }
		};

		void OnCreate(
			Device*						pDevice,
			UploadHeap*					pUploadHeap,
			ResourceViewHeaps*			pHeaps,
			DynamicBufferRing*			pDynamicBufferRing,
			StaticBufferPool*			pStaticBufferPool,
			GLTFTexturesAndBuffers*		pGLTFTexturesAndBuffers,
			SkyDome*					pSkyDome,
			bool						bUseSSAOMask,
			std::vector<VkImageView>&	shadowMapViewPool,
			GBufferRenderPass*			pRenderPass,
			AsyncPool*					pAsyncPool = NULL);

		void	OnDestroy();
		void	BuildBatchLists(std::vector<BatchList>* pSolid, std::vector<BatchList> *pTransparent, bool bWireframe = false);
		void	DrawBatchList(VkCommandBuffer commandBuffer, std::vector<BatchList> *pBatchList, bool bWireframe = false);
		void	OnUpdateWindowSizeDependentResources(VkImageView SSAO);

	private:
		GLTFTexturesAndBuffers*			m_pGLTFTexturesAndBuffers;

		ResourceViewHeaps*				m_pResourceViewHeaps;
		DynamicBufferRing*				m_pDynamicBufferRing;
		StaticBufferPool*				m_pStaticBufferPool;

		std::vector<PBRMesh>			m_meshes;
		std::vector<PBRMaterial>		m_materialsData;

		//GltfPbrPass::per_frame			m_cbPerFrame;

		PBRMaterial						m_defaultMaterial;

		Device*							m_pDevice;
		GBufferRenderPass*				m_pRenderPass;
		VkSampler						m_samplerPbr = VK_NULL_HANDLE, m_samplerShadow = VK_NULL_HANDLE;
		
		// PBR Brdf
		Texture							m_brdfLutTexture;
		VkImageView						m_brdfLutView		= VK_NULL_HANDLE;
		VkSampler						m_brdfLutSampler	= VK_NULL_HANDLE;

		void CreateDescriptorTableForMaterialTextures(PBRMaterial* tfMat, std::map<std::string, VkImageView> &texturesBase, SkyDome* pSkyDome, std::vector<VkImageView>& shadowMapViewPool, bool buseSSAOMask);
		void CreateDescriptors(int inverseMatrixBufferSize, DefineList* pAttributeDefines, PBRPrimitives* pPrimitive, bool bUseSSAOMask);
		void CreatePipeline(std::vector<VkVertexInputAttributeDescription> layout, const DefineList& defines, PBRPrimitives* pPrimitive);

	};
}