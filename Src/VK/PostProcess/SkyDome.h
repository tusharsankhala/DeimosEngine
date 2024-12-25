#pragma once

#include "PostProcPS.h"
#include "VK/Base/Texture.h"
#include "VK/Base/UploadHeap.h"

#include <vectormath/vectormath.hpp>

namespace Engine_VK
{
	class SkyDome
	{
	public:
		void					OnCreate(Device* pDevice, VkRenderPass renderPass, UploadHeap* pUploadHeap, VkFormat outFormat, 
										 ResourceViewHeaps* pHeaps, DynamicBufferRing* pDynamicBufferRing, StaticBufferPool* pStaticBufferPool,
										 const char* pDiffuseCubemap, const char* pSpecularCubemap, VkSampleCountFlagBits sampleDescCount);

		void					OnDestroy();
		void					Draw( VkCommandBuffer cmd_buf, const math::Matrix4& invViewProj);
		void					GenerateDiffuseMapFromEnvironmentMap();

		void					SetDescriptorDiff(uint32_t index, VkDescriptorSet descriptorSet);
		void					SetDescriptorSpec(uint32_t index, VkDescriptorSet descriptorSet);

		/*
		VkImageView				GetCubeDiffuseTextureView() const;
		VkImageView				GetCubeSpecularTextureView() const;
		VkSampler				GetCubeDiffuseTextureSampler() const;
		VkSampler				GetCubeDiffuseTextureSampler() const;
		*/

	private:
		Device*					m_pDevice;

		ResourceViewHeaps*		m_pResourceViewHeaps;

		Texture					m_cubeDiffuseTexture;
		Texture					m_cubeSpecularTexture;

		VkImageView				m_cubeDiffuseTextureView;
		VkImageView				m_cubeSpecularTextureView;

		VkSampler				m_samplerDiffuseCube, m_samplerSpecularCube;

		VkDescriptorSet			m_descriptorSet;
		VkDescriptorSetLayout	m_descriptorLayout;

		PostProcPS				m_skydome;

		DynamicBufferRing*		m_pDynamicBufferRing = NULL;

	};
}

