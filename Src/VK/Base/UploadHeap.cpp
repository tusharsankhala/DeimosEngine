#include "Common/stdafx.h"
#include "VK/Base/Device.h"
#include "VK/Base/UploadHeap.h"
#include "Common/Misc/Misc.h"

namespace Engine_VK
{
	//----------------------------------------------------------------------------------------
	//
	// OnCreate
	//
	//----------------------------------------------------------------------------------------

	void UploadHeap::OnCreate(Device* pDevice, SIZE_T uSize)
	{
		m_pDevice = pDevice;

		VkResult res;

		// Create command lists and allocators.
		{
			VkCommandPoolCreateInfo cmd_pool_info = {};
			cmd_pool_info.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmd_pool_info.queueFamilyIndex	= m_pDevice->GetGraphicsQueueFamilyIndex();
			cmd_pool_info.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			res								= vkCreateCommandPool( m_pDevice->GetDevice(), &cmd_pool_info, NULL, &m_commandPool );
			assert( res == VK_SUCCESS );

			VkCommandBufferAllocateInfo cmd = {};
			cmd.sType						= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmd.pNext						= NULL;
			cmd.commandPool					= m_commandPool;
			cmd.level						= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmd.commandBufferCount			= 1;
			res								= vkAllocateCommandBuffers( m_pDevice->GetDevice(), &cmd, &m_pCommandBuffer );
			assert( res == VK_SUCCESS );
		}

		// Create buffer to suballocate.
		{
			VkBufferCreateInfo buffer_info	= {};
			buffer_info.sType				= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.size				= uSize;
			buffer_info.usage				= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer_info.sharingMode			= VK_SHARING_MODE_EXCLUSIVE;
			res								= vkCreateBuffer( m_pDevice->GetDevice(), &buffer_info, NULL, &m_buffer );
			assert( res == VK_SUCCESS );

			VkMemoryRequirements mem_reqs;
			vkGetBufferMemoryRequirements( m_pDevice->GetDevice(), m_buffer, &mem_reqs );

			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize	= mem_reqs.size;
			alloc_info.memoryTypeIndex	= 0;

			bool pass = memory_type_from_properties( m_pDevice->GetPhysicalDeviceMemoryProperties(), mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex );
			assert( pass && "No mappable, coherent memory");

			res = vkAllocateMemory( m_pDevice->GetDevice(), &alloc_info, NULL, &m_deviceMemory );
			assert( res == VK_SUCCESS );

			res = vkBindBufferMemory( m_pDevice->GetDevice(), &m_buffer, m_deviceMemory, 0 );
			assert( res == VK_SUCCESS );

			res = vkMapMemory( m_pDevice->GetDevice(), m_deviceMemory, 0, mem_reqs.size, 0, (void **)&m_pDataBegin );
			assert( res == VK_SUCCESS );

			m_pDataCur = m_pDataBegin;
			m_pDataEnd = m_pDataBegin + mem_reqs.size;
		}

		// Create fence.
		{
			VkFenceCreateInfo fence_ci = {};
			fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

			res = vkCreateFence( m_pDevice->GetDevice(), &fence_ci, NULL, &m_fence );
			assert( res == VK_SUCCESS );
		}

		// Begin Command buffer.
		{
			VkCommandBufferBeginInfo cmd_buf_info = {};
			cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			res = vkBeginCommandBuffer( m_pCommandBuffer, &cmd_buf_info );
			assert(res == VK_SUCCESS);
		}
	}

	//----------------------------------------------------------------------------------------
	//
	// OnCreate
	//
	//----------------------------------------------------------------------------------------
	void UploadHeap::OnDestroy()
	{
		vkDestroyBuffer( m_pDevice->GetDevice(), m_buffer, NULL );
		vkUnmapMemory( m_pDevice->GetDevice(), m_deviceMemory );
		vkFreeMemory( m_pDevice->GetDevice(), m_deviceMemory, NULL );

		vkFreeCommandBuffers( m_pDevice->GetDevice(), m_commandPool, 1, &m_pCommandBuffer );
		vkDestroyCommandPool( m_pDevice->GetDevice(), m_commandPool, NULL );

		vkDestroyFence( m_pDevice->GetDevice(), m_fence, NULL );
	}
}