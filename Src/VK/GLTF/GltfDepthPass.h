#pragma once

#include "GLTFTexturesAndBuffers.h"

namespace Engine_VK
{
	// Material, primitive and mesh structs specific for the depth pass
	// (you might want to compare these structs with the ones used for the depth pass in GltfPbrPass.h)

	struct DepthMaterial
	{
		int							m_textureCount			= 0;
		VkDescriptorSet				m_descriptorSet			= VK_NULL_HANDLE;
		VkDescriptorSetLayout		m_descriptorSetLayout	= VK_NULL_HANDLE;

		DefineList					m_defines;
		bool						m_doubleSided			= false;
	};

	struct DepthPrimitives
	{
		Geometry 					m_geometry;
		DepthMaterial*				m_pMaterial				= NULL;
		
		VkPipeline					m_pipeline				= VK_NULL_HANDLE;
		VkPipelineLayout			m_pipelineLayout		= VK_NULL_HANDLE;

		VkDescriptorSet				m_descriptorSet			= VK_NULL_HANDLE;
		VkDescriptorSetLayout		m_descriptorSetLayout	= VK_NULL_HANDLE;
	};

	struct DepthMesh
	{
		std::vector<DepthPrimitives> m_pPrimitives;
	};

	class GltfDepthPass
	{
	public:
		struct per_frame
		{
			math::Matrix4 m_viewProj;
		};

		struct per_object
		{
			math::Matrix4 m_world;
		};

		void OnCreate(Device*		pDevice,
			VkRenderPass			renderPass,
			UploadHeap*				pUploadHeap,
			ResourceViewHeaps*		pHeaps,
			DynamicBufferRing*		pDynamicBufferRing,
			StaticBufferPool*		pStaticBufferPool,
			GLTFTexturesAndBuffers* pGLTFTexturesAndBuffers,
			AsyncPool*				pAsyncPool = NULL);
		
		void OnDestroy();
		GltfDepthPass::per_frame*	SetPerFrameConstants();
		void						Draw(VkCommandBuffer cmd_buf);

	private:
		ResourceViewHeaps*			m_pResourceViewHeaps;
		DynamicBufferRing*			m_pDynamicBufferRing;
		StaticBufferPool*			m_pStaticBufferPool;

		std::vector<DepthMesh>		m_meshes;
		std::vector<DepthMaterial>	m_materialsData;

		DepthMaterial				m_defaultMaterial;

		GLTFTexturesAndBuffers*		m_pGLTFTexturesAndBuffers;
		Device*						m_pDevice;
		VkRenderPass				m_renderPass	= VK_NULL_HANDLE;
		VkSampler					m_sampler		= VK_NULL_HANDLE;
		VkDescriptorBufferInfo		m_perFrameDesc;

		void CreateDescriptor(int inverseMatrixBufferSize, DefineList* pAtributeDefines, DepthPrimitives* pPrimitive);
		void CreatePipeline(std::vector<VkVertexInputAttributeDescription> layout, const DefineList& defines, DepthPrimitives* pPrimitive);

	};
}
