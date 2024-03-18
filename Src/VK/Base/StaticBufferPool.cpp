#include "Common/stdafx.h"
#include "Common/Misc/Misc.h"
#include "StaticBufferPool.h"

namespace Engine_VK
{
	// Allocate a IB/VB and returns a pointer to fill it + a descriptor.
	//
	bool StaticBufferPool::AllocBuffer(uint32_t numberOfElements, uint32_t strideInBytes, void** pData, VkDescriptorBufferInfo* pOut)
	{
		std::lock_guard<std::mutex> lock( m_mutex );

		uint32_t size = AlignUp( numberOfElements * strideInBytes, 256u );
		assert( m_memOffset + size < m_totalMemSize );

		*pData = ( void *)( m_pData + m_memOffset );

		pOut->buffer = m_bUseVidMem ? m_bufferVid : m_buffer;
		pOut->offset = m_memOffset;
		pOut->range = size;
		m_memOffset += size;
		return true;
	}
}