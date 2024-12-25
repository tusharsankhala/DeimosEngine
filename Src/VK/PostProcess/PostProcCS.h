#pragma once

#include "VK/Base/Device.h"
#include "VK/Base/ShaderCompilerHelper.h"

namespace Engine_VK
{
	class PostProcCS
	{
	public:
		void OnCreate(Device* pDevice,
			const std::string& shaderFileName,
			const std::string& shaderEntryPoint,
			const std::string& shaderCompilerParams,
			VkDescriptorSetLayout descriptorSetLayout,
			uint32_t dwWidth, uint32_t dwHeight, uint32_t dwDepth,
			DefineList* userDefines = 0);

		void OnDestroy();
		void Draw(VkCommandBuffer cmd_buf, VkDescriptorBufferInfo* pConstantBuffer, VkDescriptorSet descriptorSet, uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ);

	private:
		Device* m_pDevice;

		VkPipeline										m_pipeline = VK_NULL_HANDLE;
		VkPipelineLayout								m_pipelineLayout = VK_NULL_HANDLE;
	};
}