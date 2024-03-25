#pragma once

#include "VK/Base/DynamicBufferRing.h"
#include "VK/Base/CommandListRing.h"
#include "VK/Base/UploadHeap.h"
#include <ImGui/imgui.h>
#include "Common/Base/ImGuiHelper.h"

namespace Engine_VK
{
	// This is the vulkan rendering backend for the excellent ImGUI library.

	class ImGUI
	{
	public:
		void OnCreate( Device* pDevice, VkRenderPass renderPass, UploadHeap* pUploadHeap, DynamicBufferRing* pConstantBufferRing, float fontSize = 13.f ); // Initializer to not break the API.
		void OnDestroy();

		void UpdatePipeline( VkRenderPass renderPass );
		void Draw( VkCommandBuffer cmd_buf );

	private:
		Device*											m_pDevice				= nullptr;
		DynamicBufferRing*								m_pConstBuf				= nullptr;

		VkImage											m_pTexture2D			= VK_NULL_HANDLE;
		VmaAllocation									m_imageAlloc			= VK_NULL_HANDLE;
		VkDescriptorBufferInfo							m_geometry;
		VkPipelineLayout								m_pipelineLayout		= VK_NULL_HANDLE;
		VkDescriptorPool								m_descriptorPool		= VK_NULL_HANDLE;
		VkPipeline										m_pipeline				= VK_NULL_HANDLE;
		VkDescriptorSet									m_descriptorSet[128];
		uint32_t										m_currentDescriptorSet;
		VkSampler										m_sampler				= VK_NULL_HANDLE;
		VkImageView										m_pTextureSRV			= VK_NULL_HANDLE;
		VkDescriptorSetLayout							m_desc_layout;
		std::vector<VkPipelineShaderStageCreateInfo>	m_shaderStages;
	};

}