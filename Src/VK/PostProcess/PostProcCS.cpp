#include "PostProcCS.h"

namespace Engine_VK
{

	void PostProcCS::OnCreate(Device* pDevice,
		const std::string& shaderFileName,
		const std::string& shaderEntryPoint,
		const std::string& shaderCompilerParams,
		VkDescriptorSetLayout descriptorSetLayout,
		uint32_t dwWidth, uint32_t dwHeight, uint32_t dwDepth,
		DefineList* userDefines)
	{
		m_pDevice = pDevice;

		VkResult res;

		// Compile shaders
		VkPipelineShaderStageCreateInfo computeShader;
		DefineList defines;

	}

	void PostProcCS::OnDestroy()
	{
		vkDestroyPipeline(m_pDevice->GetDevice(), m_pipeline, nullptr);
		vkDestroyPipelineLayout(m_pDevice->GetDevice(), m_pipelineLayout, nullptr);
	}

	void PostProcCS::Draw(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet descriptorSet, uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ)
	{

	}
}