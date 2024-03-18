#pragma once

#include "VK/Base/Device.h"
#include "Common/Misc/Ring.h"
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

namespace Engine_VK
{
    // This class mimics the behaviour or the DX11 dynamic buffers. I can hold uniforms, index and vertex buffers.
    // It does so by suballocating memory from a huge buffer. The buffer is used in a ring fashion.  
    // Allocated memory is taken from the tail, freed memory makes the head advance;
    // See 'ring.h' to get more details on the ring buffer.
    //
    // The class knows when to free memory by just knowing:
    //    1) the amount of memory used per frame
    //    2) the number of backbuffers 
    //    3) When a new frame just started ( indicated by OnBeginFrame() )
    //         - This will free the data of the oldest frame so it can be reused for the new frame
    //
    // Note than in this ring an allocated chuck of memory has to be contiguous in memory, that is it cannot spawn accross the tail and the head.
    // This class takes care of that.

    class DynamicBufferRing
    {
        public:
            VkResult    OnCreate( Device* pDevice, uint32_t numberOfBackBuffers, uint32_t memTotalSize, char* name = NULL );
            void        OnDestroy();

    private:
        Device*         m_pDevice;
        uint32_t        m_memTotalSize;
        RingWithTabs    m_mem;
        char*           m_pData = nullptr;
        VkBuffer        m_buffer;

#ifdef USE_VMA
        VmaAllocation   m_bufferAlloc   = VK_NULL_HANDLE;
#else
        VkDeviceMemory  m_deviceMemory  = VK_NULL_HANDLE;
#endif
    };
}
