#pragma once

#include "Common/Misc/Ring.h"

namespace Engine_VK
{
    // This class, on creation allocates a number of command lists. Using a ring buffer
    // these commandLists are recycled when they are no longer used by the GPU. See the 
    // 'ring.h' for more details on allocation and recycling 
    //
    class CommandListRing
    {
    public:
        void OnCreate( Device* pDevice, uint32_t numberOfBackBuffers, uint32_t commandListPerFrame, bool compute = false );
        void OnDestroy();
        void OnBeginFrame();
        VkCommandBuffer GetNewCommandList();
        VkCommandPool GetPool() { return m_pCommandBuffers->m_commandPool; }

    private:
        uint32_t m_frameIndex;
        uint32_t m_numberOfAllocators;
        uint32_t m_commandListsPerBackBuffer;

        Device* m_pDevice;

        struct CommandBuffersPerFrame
        {
            VkCommandPool       m_commandPool;
            VkCommandBuffer*    m_pCommandBuffer;
            uint32_t            m_usedCls;
        } *m_pCommandBuffers, *m_pCurrentFrame;
    };
}