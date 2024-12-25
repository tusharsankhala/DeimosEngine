#include "Common/stdafx.h"
#include "VK/Base/Helper.h"
#include "VK/Extensions/ExtDebugUtils.h"

namespace Engine_VK
{
	// this is when you need to clear the attachment, for example when you are not rendering the full screen.
	//
	void AttachClearBeforeUse( VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout intialLayout, VkImageLayout finalLayout, VkAttachmentDescription* pAttachDesc )
	{
		pAttachDesc->format			= format;
		pAttachDesc->samples		= sampleCount;
		pAttachDesc->loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
		pAttachDesc->storeOp		= VK_ATTACHMENT_STORE_OP_NONE;
		pAttachDesc->stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		pAttachDesc->stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		pAttachDesc->initialLayout	= intialLayout;
		pAttachDesc->finalLayout	= finalLayout;
		pAttachDesc->flags			= 0;
	}
	
	// Attachment where we will be using Alpha Blending, this means we care about the previous contents.
	void AttachBlending(VkFormat format, VkSampleCountFlagBits sampleCount, VkImageLayout intialLayout, VkImageLayout finalLayout, VkAttachmentDescription* pAttachDesc )
	{
		pAttachDesc->format = format;
		pAttachDesc->samples = sampleCount;
		pAttachDesc->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		pAttachDesc->storeOp = VK_ATTACHMENT_STORE_OP_NONE;
		pAttachDesc->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		pAttachDesc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		pAttachDesc->initialLayout = intialLayout;
		pAttachDesc->finalLayout = finalLayout;
		pAttachDesc->flags = 0;
	}

	VkRenderPass CreateRenderPassOptimal(VkDevice device, uint32_t colorAttachments, VkAttachmentDescription* pColorAttachments, VkAttachmentDescription* pDepthAttachment )
	{
		// We need to put all the color and the depth attachments in the same buffer.
		//
		VkAttachmentDescription attachments[10];

		// Making sure that we don't overflow the scratch buffer above.
		assert( colorAttachments < 10 );

		memcpy( attachments, pColorAttachments, sizeof( VkAttachmentDescription ) * colorAttachments );
		if( pDepthAttachment != NULL )
			memcpy( &attachments[ colorAttachments], pDepthAttachment, sizeof( VkAttachmentDescription ));

		// Create references for the attachments.
		//
		VkAttachmentReference color_reference[10];
		for( uint32_t i=0; i< colorAttachments; ++i )
			color_reference[i] = { i, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkAttachmentReference depth_references = { colorAttachments, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		// Create subpass.
		//
		VkSubpassDescription subpass	= {};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags					= 0;
		subpass.inputAttachmentCount	= 0;
		subpass.pInputAttachments		= NULL;
		subpass.colorAttachmentCount	= colorAttachments;
		subpass.pColorAttachments		= color_reference;
		subpass.pResolveAttachments		= NULL;
		subpass.pDepthStencilAttachment	= (pDepthAttachment) ? &depth_references : NULL;
		subpass.preserveAttachmentCount	= 0;
		subpass.pPreserveAttachments	= NULL;

		VkSubpassDependency dep	= {};
		dep.dependencyFlags				= 0;
		dep.dstAccessMask				= VK_ACCESS_SHADER_READ_BIT |
											((colorAttachments) ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : 0 ) |
											((pDepthAttachment) ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT : 0);
		dep.dstStageMask				= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
											((colorAttachments) ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : 0) |
											((pDepthAttachment) ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT : 0);
		dep.dstStageMask				= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
											((colorAttachments) ? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT : 0) |
											((pDepthAttachment) ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT : 0);
		dep.dstSubpass					= VK_SUBPASS_EXTERNAL;
		dep.srcAccessMask				= ((colorAttachments) ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : 0) |	((pDepthAttachment) ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : 0);
		dep.srcStageMask				= ((colorAttachments) ? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT : 0 ) | ((pDepthAttachment) ? VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT : 0);
		dep.srcSubpass = 0;

		// Create render pass.
		//
		VkRenderPassCreateInfo rp_info = {};
		rp_info.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext					= NULL;
		rp_info.attachmentCount			= colorAttachments;
		if( pDepthAttachment != NULL )
			rp_info.attachmentCount++;
		rp_info.pAttachments			= attachments;
		rp_info.subpassCount			= 1;
		rp_info.pSubpasses				= &subpass;
		rp_info.dependencyCount			= 1;
		rp_info.pDependencies			= &dep;

		VkRenderPass render_pass;
		VkResult res = vkCreateRenderPass( device, &rp_info, NULL, &render_pass );
		assert( res == VK_SUCCESS );

		SetResourceName ( device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)render_pass, "CreateRenderPassOptimal" );

		return render_pass;
	}

	// Sets the viewport and the scissor to a fixed height and width.
	//
	void SetViewportAndScissor(VkCommandBuffer cmd_buf, uint32_t topX, uint32_t topY, uint32_t width, uint32_t height)
	{
		VkViewport viewport;
		viewport.x			= static_cast<float>(topX);
		viewport.y			= static_cast<float>(topY) + static_cast<float>(height);
		viewport.width		= static_cast<float>(width);
		viewport.height		= static_cast<float>(height);
		viewport.minDepth	= (float)0.0f;
		viewport.maxDepth	= (float)1.0f;
		vkCmdSetViewport( cmd_buf, 0, 1, &viewport );

		VkRect2D scissor;
		scissor.extent.width	= (uint32_t)(width);
		scissor.extent.height	= (uint32_t)(height);
		scissor.offset.x		= topX;
		scissor.offset.y		= topY;
		vkCmdSetScissor( cmd_buf, 0, 1, &scissor );
	}

	void SetDescriptorSet(VkDevice device, uint32_t index, VkImageView imageView, VkImageLayout imageLayout, VkSampler* pSampler, VkDescriptorSet descriptorSet)
	{
		VkDescriptorImageInfo desc_image;
		desc_image.sampler		= (pSampler == NULL) ? VK_NULL_HANDLE : *pSampler;
		desc_image.imageView	= imageView;
		desc_image.imageLayout	= imageLayout;

		VkWriteDescriptorSet write;
		write = {};
		write.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext				= NULL;
		write.dstSet			= descriptorSet;
		write.descriptorCount	= 1;
		write.descriptorType	= (pSampler == NULL) ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.dstBinding		= index;
		write.dstArrayElement	= 0;
		
		vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
	}

	void SetDescriptorSet(VkDevice device, uint32_t index, VkImageView imageView, VkDescriptorSet descriptorSet)
	{
		VkDescriptorImageInfo desc_image;
		desc_image.sampler = VK_NULL_HANDLE;
		desc_image.imageView = imageView;
		desc_image.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

		VkWriteDescriptorSet write;
		write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = NULL;
		write.dstSet = descriptorSet;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		write.pImageInfo = &desc_image;
		write.dstBinding = index;
		write.dstArrayElement = 0;

		vkUpdateDescriptorSets(device, 1, &write, 0, NULL);
	}

	void SetDescriptorSet(VkDevice device, uint32_t index, VkImageView imageView, VkSampler* pSampler, VkDescriptorSet descriptorSet)
	{
		SetDescriptorSet(device, index, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, pSampler, descriptorSet);
	}

	VkRenderPass SimpleColorWriteRenderPass(VkDevice device, VkImageLayout initialLayout, VkImageLayout passLayout, VkImageLayout finalLayout)
	{
		// Color RT.
		VkAttachmentDescription attachments[1];
		attachments[0].format			= VK_FORMAT_R16G16B16A16_SFLOAT;
		attachments[0].samples			= VK_SAMPLE_COUNT_1_BIT;
		attachments[0].loadOp			= VK_ATTACHMENT_LOAD_OP_DONT_CARE;		// We don't care about the previous contents. this is for a full screen pass with no blending.
		attachments[0].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout	= initialLayout;
		attachments[0].finalLayout		= finalLayout;
		attachments[0].flags			= 0;

		VkAttachmentReference color_reference = { 0, passLayout };

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags					= 0;
		subpass.inputAttachmentCount	= 0;
		subpass.colorAttachmentCount	= 1;
		subpass.pColorAttachments		= &color_reference;
		subpass.pResolveAttachments		= NULL;
		subpass.pDepthStencilAttachment = NULL;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments	= NULL;

		VkSubpassDependency dep = {};
		dep.dependencyFlags				= 0;
		dep.dstAccessMask				= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
		dep.dstStageMask				= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dep.dstSubpass					= VK_SUBPASS_EXTERNAL;
		dep.srcAccessMask				= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
		dep.srcStageMask				= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dep.srcSubpass					= 0;

		VkRenderPassCreateInfo rp_info = {};
		rp_info.sType					= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rp_info.pNext					= NULL;
		rp_info.attachmentCount			= 1;
		rp_info.pAttachments			= attachments;
		rp_info.subpassCount			= 1;
		rp_info.pSubpasses				= &subpass;
		rp_info.dependencyCount			= 1;
		rp_info.pDependencies			= &dep;

		VkRenderPass renderPass;
		VkResult res = vkCreateRenderPass(device, &rp_info, NULL, &renderPass);
		assert(res == VK_SUCCESS);

		SetResourceName(device, VK_OBJECT_TYPE_RENDER_PASS, (uint64_t)renderPass, "SimpleColorWriteRenderPass");

		return renderPass;
	}

	VkFramebuffer CreateFrameBuffer( VkDevice device, VkRenderPass renderPass, const std::vector<VkImageView>* pAttachments, uint32_t width, uint32_t height )
	{
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType	= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.layers	= 1;
		framebuffer_info.pNext	= NULL;
		framebuffer_info.width	= width;
		framebuffer_info.height	= height;

		VkFramebuffer framebuffer;

		VkResult res;
		framebuffer_info.renderPass			= renderPass;
		framebuffer_info.pAttachments		= pAttachments->data();
		framebuffer_info.attachmentCount	= (uint32_t)pAttachments->size();
		res = vkCreateFramebuffer( device, &framebuffer_info, NULL, &framebuffer );
		assert( res == VK_SUCCESS );

		SetResourceName( device, VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t)framebuffer, "HelperCreateFramebuffer" );

		return framebuffer;
	}

	void BeginRenderPass( VkCommandBuffer commandList, VkRenderPass renderPass, VkFramebuffer framebuffer, const std::vector<VkClearValue>* pClearValues, uint32_t width, uint32_t height )
	{
		VkRenderPassBeginInfo rp_begin;
		rp_begin.sType						= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rp_begin.pNext						= NULL;
		rp_begin.renderPass					= renderPass;
		rp_begin.framebuffer				= framebuffer;
		rp_begin.renderArea.offset.x		= 0;
		rp_begin.renderArea.offset.y		= 0;
		rp_begin.renderArea.extent.width	= width;
		rp_begin.renderArea.extent.height	= height;
		rp_begin.pClearValues				= pClearValues->data();
		rp_begin.clearValueCount			= (uint32_t)pClearValues->size();
		vkCmdBeginRenderPass( commandList, &rp_begin, VK_SUBPASS_CONTENTS_INLINE );
	}

}