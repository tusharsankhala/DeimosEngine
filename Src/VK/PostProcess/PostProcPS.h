#pragma once

#include "VK/Base/StaticBufferPool.h"
#include "VK/Base/DynamicBufferRing.h"
#include "VK/Base/ResourceViewHeaps.h"

namespace Engine_VK
{
	class PostProcPS
	{
	public:
		void OnCreate(	Device* pDevice, VkRenderPass renderPass,
						const std::string& shaderFileName,
						const std::string& shaderEntryPoint, const std::string& shaderCompilerParams,
						StaticBufferPool* pStaticBufferPool,
						DynamicBufferRing* pDynamicBufferRing,
						VkDescriptorSetLayout descriptorSetLayout,
						VkPipelineColorBlendStateCreateInfo* pBlendDesc = NULL,
						VkSampleCountFlagBits sampleDescCount = VK_SAMPLE_COUNT_1_BIT);
		void OnDestroy();
		void UpdatePipeline( VkRenderPass renderPass, VkPipelineColorBlendStateCreateInfo* pBlendDesc = NULL,
							 VkSampleCountFlagBits sampleDescCount = VK_SAMPLE_COUNT_1_BIT);

		void Draw(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet descriptorSet = NULL);
	private:
		Device*											m_pDevice;
		std::vector<VkPipelineShaderStageCreateInfo>	m_shaderStages;
		std::string										m_fragmentShaderName;

		// all bounding boxes of all the meshes use the same geometry, shaders and pipelines.
		uint32_t										m_numIndices;
		VkIndexType										m_indexType;
		VkDescriptorBufferInfo							m_IBV;

		VkPipeline										m_pipeline			= VK_NULL_HANDLE;
		VkRenderPass									m_renderPass		= VK_NULL_HANDLE;
		VkPipelineLayout								m_pipelineLayout	= VK_NULL_HANDLE;

	};
}

