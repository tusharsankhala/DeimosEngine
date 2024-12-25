#pragma once


#include "VK/Base/Texture.h"
#include "Common/ShaderCompiler/ShaderCompiler.h"
#include "VK/Base/StaticBufferPool.h"
#include "VK/Base/DynamicBufferRing.h"
#include <json/json.h>

class GLTFCommon;

using json = nlohmann::json;

namespace Engine_VK
{
	
	// This class takes a GltfCommon class (that holds all the non-GPU specific data) as an input and loads all the GPU specific data.
	//
	struct Geometry
	{
		VkIndexType									m_indexType;
		uint32_t									m_numIndices;
		VkDescriptorBufferInfo						m_IBV;
		std::vector<VkDescriptorBufferInfo>			m_VBV;
	};

	
	class GLTFTexturesAndBuffers
	{
		
		Device*										m_pDevice;
		UploadHeap*									m_pUploadHeap;

		const json*									m_pTextureNodes;

		std::vector<Texture>						m_textures;
		std::vector<VkImageView>					m_textureViews;

		std::map<int, VkDescriptorBufferInfo>		m_skeletonMatricesBuffer;

		StaticBufferPool*							m_pStaticBufferPool;
		DynamicBufferRing*							m_pDynamicBufferRing;

		// maps GLTF ids into views.
		std::map<int, VkDescriptorBufferInfo>		m_vertexBufferMap;
		std::map<int, VkDescriptorBufferInfo>		m_indexBufferMap;
		
	public:

		GLTFCommon*									m_pGLTFCommon;

		VkDescriptorBufferInfo						m_perFrameConstants;
		
		bool										OnCreate(Device* pDevice, GLTFCommon* pGLTFCommon, UploadHeap* pUploadHeap, StaticBufferPool* pStaticBufferPool, DynamicBufferRing* pDynamicBufferRing);
		void										LoadTextures(AsyncPool* pAsyncPool = NULL);
		void										LoadGeometry();
		void										OnDestroy();

		void										CreateIndexBuffer(int indexBufferId, uint32_t* pNumIndices, VkIndexType* pIndexType, VkDescriptorBufferInfo* pIBV);
		void										CreateGeometry(int indexBufferId, std::vector<int>& vertexBufferIds, Geometry* pGeometry);
		void										CreateGeometry(const json& primitive, const std::vector<std::string> requiredAttributes, std::vector<VkVertexInputAttributeDescription>& layout, DefineList& defines, Geometry* pGeometry);

		VkImageView									GetTextureViewByID(int id);

		VkDescriptorBufferInfo*						GetSkinningMatricesBuffer(int skinIndex);
		void										SetSkinningMatricesForSkeleton();
		void										SetPerFrameConstants();

		
	};

}
