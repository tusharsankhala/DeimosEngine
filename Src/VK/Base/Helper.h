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

	// Creates a render pass that will discard the contents of the rener target.
	//
	VkRenderPass SimpleColorWriteRenderPass(VkDevice device, VkImageLayout initialLayout, VkImageLayout passLayout, VkImageLayout finalLayout);

	// Sets the i-th Descriptor set entry to use a given image view + sampler. The sampler can be null is a static one is being used.
	void			SetDescriptorSet(VkDevice device, uint32_t index, VkImageView imageView, VkSampler* pSampler, VkDescriptorSet descriptorSet );
	void			SetDescriptorSet(VkDevice device, uint32_t index, VkImageView imageView, VkDescriptorSet descriptorSet);
	VkFramebuffer	CreateFrameBuffer( VkDevice device, VkRenderPass renderPass, const std::vector<VkImageView>* pAttachments, uint32_t width, uint32_t height );
}
