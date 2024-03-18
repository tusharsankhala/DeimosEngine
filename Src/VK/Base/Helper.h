#pragma once

#include <vulkan/vulkan.h>

namespace Engine_VK
{
	void			AttachClearBeforeUse( VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout intialLayout, VkImageLayout finalLayout, VkAttachmentDescription* pAttachDesc );
	void			AttachBlending( VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout intialLayout, VkImageLayout finalLayout, VkAttachmentDescription* pAttachDesc );
	VkRenderPass	CreateRenderPassOptimal( VkDevice device, uint32_t colorAttachments, VkAttachmentDescription* pColorAttachments, VkAttachmentDescription* pDepthAttachment );

	// Sets the viewport and the scissor to a fixed height and width.
	//
	void			SetViewportAndScissor( VkCommandBuffer cmd_buf, uint32_t topX, uint32_t topY, uint32_t width, uint32_t height );

	VkFramebuffer	CreateFrameBuffer( VkDevice device, VkRenderPass renderPass, const std::vector<VkImageView>* pAttachments, uint32_t width, uint32_t height );
}
