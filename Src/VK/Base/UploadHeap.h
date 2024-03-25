#pragma once

#include "VK/Base/Device.h"
#include "Common/Misc/Async.h"

namespace Engine_VK
{
	//
	// This class shows the most efficient way to upload resources to the GPU memory. 
	// The idea is to create just one upload heap and suballocate memory from it.
	// For convenience this class comes with it's own command list & submit (FlushAndFinish)
	//

	class UploadHeap
	{
		Sync allocating, flushing;
		struct COPY
		{
			VkImage							m_image;
			VkBufferImageCopy				m_bufferImageCopy;
		};

		std::vector<COPY> m_copies;

		std::vector<VkImageMemoryBarrier>	m_toPreBarrier;
		std::vector<VkImageMemoryBarrier>	m_toPostBarrier;

		std::mutex							m_mutex;

	public:
		void			OnCreate( Device* pDevice, SIZE_T uSize );
		void			OnDestroy();

		UINT8*			Suballocate( SIZE_T uSize, UINT64 uAlign );
		UINT8*			BeginSuballocate( SIZE_T uSize, UINT64 uAlign );
		void			EndSuballocate();
		UINT8*			BasePtr() { return m_pDataBegin; }
		VkBuffer		GetResource() { return m_buffer; }
		VkCommandBuffer	GetCommandList() { return m_commandBuffer; }

		void			Flush();
		void			FlushAndFinish( bool bDoBarriers = false );


	private:

		Device*								m_pDevice;

		VkCommandPool						m_commandPool;
		VkCommandBuffer						m_commandBuffer;

		VkBuffer							m_buffer;
		VkDeviceMemory						m_deviceMemory;

		VkFence								m_fence;

		UINT8*								m_pDataBegin	= nullptr;			// Starting position of the upload heap.
		UINT8*								m_pDataCur		= nullptr;			// Current position of the upload heap.
		UINT8*								m_pDataEnd		= nullptr;			// Ending position of the upload heap.
	};
}