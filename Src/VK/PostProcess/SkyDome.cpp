#include "Common/stdafx.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Base/Helper.h"

#include "SkyDome.h"


namespace Engine_VK
{
	void SkyDome::OnCreate(Device* pDevice, VkRenderPass renderPass, UploadHeap* pUploadHeap, VkFormat outFormat,
		ResourceViewHeaps* pHeaps, DynamicBufferRing* pDynamicBufferRing, StaticBufferPool* pStaticBufferPool,
		const char* pDiffuseCubemap, const char* pSpecularCubemap, VkSampleCountFlagBits sampleDescCount)
	{

	}

	void SkyDome::OnDestroy()
	{
		m_skydome.OnDestroy();
		vkDestroyDescriptorSetLayout(m_pDevice->GetDevice(), m_descriptorLayout, NULL);

		vkDestroySampler(m_pDevice->GetDevice(), m_samplerDiffuseCube, nullptr);
		vkDestroySampler(m_pDevice->GetDevice(), m_samplerSpecularCube, nullptr);

		vkDestroyImageView(m_pDevice->GetDevice(), m_cubeDiffuseTextureView, nullptr);
		vkDestroyImageView(m_pDevice->GetDevice(), m_cubeSpecularTextureView, nullptr);

		m_pResourceViewHeaps->FreeDescriptor(m_descriptorSet);

		m_cubeDiffuseTexture.OnDestroy();
		m_cubeSpecularTexture.OnDestroy();
	}

	void SkyDome::Draw(VkCommandBuffer cmd_buf, const math::Matrix4& invViewProj)
	{
		SetPerfMarkerBegin(cmd_buf, "Skydome cube");

		math::Matrix4*				cbPerDraw;
		VkDescriptorBufferInfo		constantBuffer;
		m_pDynamicBufferRing->AllocConstantBuffer(sizeof(math::Matrix4), (void**)&cbPerDraw, &constantBuffer);
		*cbPerDraw = invViewProj;

		m_skydome.Draw(cmd_buf, &constantBuffer, m_descriptorSet);

		SetPerfMarkerEnd(cmd_buf);
	}

	void SkyDome::GenerateDiffuseMapFromEnvironmentMap()
	{ }

	void SkyDome::SetDescriptorDiff(uint32_t index, VkDescriptorSet descriptorSet)
	{
		SetDescriptorSet(m_pDevice->GetDevice(), index, m_cubeDiffuseTextureView, &m_samplerDiffuseCube, descriptorSet);
	}

	void SkyDome::SetDescriptorSpec(uint32_t index, VkDescriptorSet descriptorSet)
	{
		SetDescriptorSet(m_pDevice->GetDevice(), index, m_cubeSpecularTextureView, &m_samplerSpecularCube, descriptorSet);
	}

	/*
	// Sampler and TextureView getter.
	VkImageView SkyDome::GetCubeDiffuseTextureView() const
	{
		return m_cubeDiffuseTextureView;
	}

	VkImageView	SkyDome::GetCubeSpecularTextureView() const
	{
		return m_cubeSpecularTextureView;
	}

	VkSampler SkyDome::GetCubeDiffuseTextureSampler() const
	{
		return m_samplerDiffuseCube;
	}

	VkSampler SkyDome::GetCubeDiffuseTextureSampler() const
	{
		return m_samplerSpecularCube;
	}
	*/
}
