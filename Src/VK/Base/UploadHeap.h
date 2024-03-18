#pragma once

#include "VK/Base/Device.h"

namespace Engine_VK
{
	//
	// This class shows the most efficient way to upload resources to the GPU memory. 
	// The idea is to create just one upload heap and suballocate memory from it.
	// For convenience this class comes with it's own command list & submit (FlushAndFinish)
	//

	class UploadHeap
	{
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
		void OnCreate( Device* pDevice, SIZE_T uSize );
		void OnDestroy();
	private:

		Device*								m_pDevice;

		VkCommandPool						m_commandPool;
		VkCommandBuffer						m_pCommandBuffer;

		VkBuffer							m_buffer;
		VkDeviceMemory						m_deviceMemory;

		VkFence								m_fence;

		UINT8*								m_pDataBegin	= nullptr;			// Starting position of the upload heap.
		UINT8*								m_pDataCur		= nullptr;			// Current position of the upload heap.
		UINT8*								m_pDataEnd		= nullptr;			// Ending position of the upload heap.
	};
}