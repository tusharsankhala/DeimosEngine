#include "VK/Base/DynamicBufferRing.h"
#include "VK/Base/StaticBufferPool.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Base/Texture.h"
#include "VK/Base/Imgui.h"
#include "VK/Base/Helper.h"

#include "VK/PostProcess/DownSamplePS.h"

namespace Engine_VK
{
	void DownSamplePS::OnCreate(Device* pDevice, ResourceViewHeaps* pResourceViewHeaps, DynamicBufferRing* m_pConstantBufferRing, StaticBufferPool* pStaticBufferPool, VkFormat outFormat)
	{
		m_pDevice				= pDevice;
		m_outFormat				= outFormat;
		m_pStaticBufferPool		= pStaticBufferPool;
		m_pResourceViewHeaps	= pResourceViewHeaps;
		m_pConstantBufferRing	= m_pConstantBufferRing;

		// Create Descriptor Set Layout, the shader need a uniform dynamic buffer and a texture + sampler
		// The Descriptor Sets will be created and initialized once we know the input to the shader,
		// that happens in OnCreateWindowSizeDependentResources.
		{
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings(2);
			layoutBindings[0].binding				= 0;
			layoutBindings[0].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			layoutBindings[0].descriptorCount		= 1;
			layoutBindings[0].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBindings[0].pImmutableSamplers	= NULL;

			layoutBindings[0].binding				= 0;
			layoutBindings[0].descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBindings[0].descriptorCount		= 1;
			layoutBindings[0].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBindings[0].pImmutableSamplers	= NULL;

			VkDescriptorSetLayoutCreateInfo	descriptor_layout = {};
			descriptor_layout.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptor_layout.pNext					= NULL;
			descriptor_layout.bindingCount			= (uint32_t)layoutBindings.size();
			descriptor_layout.pBindings				= layoutBindings.data();

			VkResult res = vkCreateDescriptorSetLayout(pDevice->GetDevice(), &descriptor_layout, NULL, &m_descriptorSetLayout);
			assert(res == VK_SUCCESS);
		}

		// In Render Pass.
		//
		m_in = SimpleColorWriteRenderPass(pDevice->GetDevice(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// The sampler we want to use for downsampling, all linear
		//
		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.minLod = -1000;
			info.maxLod = 1000;
			info.maxAnisotropy = 1.0f;
			VkResult res = vkCreateSampler(pDevice->GetDevice(), &info, NULL, &m_sampler);
			assert(res == VK_SUCCESS);
		}

		// Use helper class to create the mip chain.
		m_downscale.OnCreate(pDevice, m_in, "DownSamplePS.glsl", "main", "", pStaticBufferPool, m_pConstantBufferRing, m_descriptorSetLayout);

		// Allocate descriptors for the mip chain.
		//
		for (int i = 0; i < DOWNSAMPLEPS_MAX_MIP_LEVELS; ++i)
		{
			m_pResourceViewHeaps->AllocDescriptor(m_descriptorSetLayout, &m_mip[i].descriptorSet);
		}
	}

	void DownSamplePS::OnDestroy()
	{
		for (int i = 0; i < DOWNSAMPLEPS_MAX_MIP_LEVELS; ++i)
		{
			m_pResourceViewHeaps->FreeDescriptor(m_mip[i].descriptorSet);
		}

		m_downscale.OnDestroy();
		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_descriptorSetLayout, NULL);
		vkDestroySampler(m_pDevice->GetDevice(), m_sampler, nullptr);

		vkDestroyRenderPass(m_pDevice->GetDevice(), m_in, NULL);
	}

	void DownSamplePS::OnDestroyWindowSizeDependentResources()
	{
		for (int i = 0; i < m_mipCount; ++i)
		{
			vkDestroyImageView(m_pDevice->GetDevice(), m_mip[i].m_SRV, NULL);
			vkDestroyImageView(m_pDevice->GetDevice(), m_mip[i].RTV, NULL);
			vkDestroyFramebuffer(m_pDevice->GetDevice(), m_mip[i].frameBuffer, NULL);
		}

		m_result.OnDestroy();	
	}

	void DownSamplePS::OnCreateWindowSizeDependentResources(uint32_t width, uint32_t height, Texture* pInput, int mips)
	{

	}
	

	void DownSamplePS::Draw(VkCommandBuffer cmd_buf)
	{
		SetPerfMarkerBegin(cmd_buf, "Downsample");

		// downsample
		//
		for (int i = 0; i < m_mipCount; ++i)
		{
			VkRenderPassBeginInfo rp_begin = {};
			rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			rp_begin.pNext = NULL;
			rp_begin.renderPass = m_in;
			rp_begin.framebuffer = m_mip[i].frameBuffer;
			rp_begin.renderArea.offset.x = 0;
			rp_begin.renderArea.offset.y = 0;
			rp_begin.renderArea.extent.width = m_width >> (i + 1);
			rp_begin.renderArea.extent.height = m_height >> (i + 1);
			rp_begin.pClearValues = NULL;
			vkCmdBeginRenderPass(cmd_buf, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
			SetViewportAndScissor(cmd_buf, 0, 0, m_width >> (i + 1), m_height >> (i + 1));

			cbDownscale* data;
			VkDescriptorBufferInfo constantBuffer;
			m_pConstantBufferRing->AllocConstantBuffer(sizeof(cbDownscale), (void**)&data, &constantBuffer);
			data->invWidth = 1.0f / (float)(m_width >> i);
			data->invHeight = 1.0f / (float)(m_height >> i);
			data->mipLevel = i;

			m_downscale.Draw(cmd_buf, &constantBuffer, m_mip[i].descriptorSet);

			vkCmdEndRenderPass(cmd_buf);
		}

		SetPerfMarkerEnd(cmd_buf);
	}
	

	void DownSamplePS::Gui()
	{
		bool opened = true;
		ImGui::Begin("DownSamplePS", &opened);

		for (int i = 0; i < m_mipCount; ++i)
		{
			ImGui::Image((ImTextureID)m_mip[i].m_SRV, ImVec2(320 / 2, 180 / 2));
		}

		ImGui::End();
	}
}