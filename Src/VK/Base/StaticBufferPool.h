#pragma once

#include "VK/Base/Device.h"
#include "VK/Base/ResourceViewHeaps.h"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

namespace Engine_VK
{
	// Simulate DX11 style static buffers. For dynamic buffers see 'DynamicBufferRingDX12.h'
	//
	// This class allows suballocating small chuncks of memory from a buge buffer that is allocated on creation
	// This class is specialized in vertex and index buffers.
	//  
	class StaticBufferPool
	{
	public:
		VkResult		OnCreate( Device* pDevice, uint32_t totalMemSize, bool bUseVidMem, const char* name );
		void			OnDestroy();

		// Allocate a IB/VB and returns a pointer to fill it + a descriptor.
		//
		bool			AllocBuffer( uint32_t numberOfVertices, uint32_t strideInBytes, void** pData,	VkDescriptorBufferInfo* pOut );

		// Allocate a IB/VB and fill it with pInitData, returns a descriptor.
		//
		bool			AllocBuffer( uint32_t numberOfIndices, uint32_t strideInBytes, const void* pInitData, VkDescriptorBufferInfo* pOut);

		// If using vidmem this kicks the upload from the upload heap to the video mem.
		void			UploadData( VkCommandBuffer cmd_buf );

		// If using vidmem frees the upload heap.
		void			FreeUploadHeap();

	private:
		Device*			m_pDevice;
		std::mutex		m_mutex				= {};

		bool			m_bUseVidMem		= true;

		char*			m_pData				= nullptr;		
		uint32_t		m_memOffset			= 0;
		uint32_t		m_totalMemSize		= 0;

		VkBuffer		m_buffer;
		VkBuffer		m_bufferVid;

#ifdef  USE_VMA
		VmaAllocation	m_bufferAlloc		= VK_NULL_HANDLE;
		VmaAllocation	m_bufferAllocVid	= VK_NULL_HANDLE;
#else
		VmaDeviceMemory	m_deviceMemory		= VK_NULL_HANDLE;
		VmaDeviceMemory	m_deviceMemoryVid	= VK_NULL_HANDLE;
#endif //  USE_VMA

	};

}