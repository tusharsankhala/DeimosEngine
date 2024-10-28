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
			res								= vkAllocateCommandBuffers( m_pDevice->GetDevice(), &cmd, &m_commandBuffer );
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

			VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = m_pDevice->GetPhysicalDeviceMemoryProperties();
			bool pass = memory_type_from_properties( physicalDeviceMemoryProperties, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex );
			assert( pass && "No mappable, coherent memory");

			res = vkAllocateMemory( m_pDevice->GetDevice(), &alloc_info, NULL, &m_deviceMemory );
			assert( res == VK_SUCCESS );

			res = vkBindBufferMemory( m_pDevice->GetDevice(), m_buffer, m_deviceMemory, 0 );
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

			res = vkBeginCommandBuffer( m_commandBuffer, &cmd_buf_info );
			assert(res == VK_SUCCESS);
		}
	}

	//----------------------------------------------------------------------------------------
	//
	// OnDestroy
	//
	//----------------------------------------------------------------------------------------
	void UploadHeap::OnDestroy()
	{
		vkDestroyBuffer( m_pDevice->GetDevice(), m_buffer, NULL );
		vkUnmapMemory( m_pDevice->GetDevice(), m_deviceMemory );
		vkFreeMemory( m_pDevice->GetDevice(), m_deviceMemory, NULL );

		vkFreeCommandBuffers( m_pDevice->GetDevice(), m_commandPool, 1, &m_commandBuffer );
		vkDestroyCommandPool( m_pDevice->GetDevice(), m_commandPool, NULL );

		vkDestroyFence( m_pDevice->GetDevice(), m_fence, NULL );
	}

	//--------------------------------------------------------------------------------------
	//
	// SuballocateFromUploadHeap
	//
	//--------------------------------------------------------------------------------------
	UINT8* UploadHeap::Suballocate( SIZE_T uSize, UINT64 uAlign )
	{
		// Wait until we are done flushing the heap.
		flushing.Wait();

		UINT8* pRet = NULL;

		{
			std::unique_lock<std::mutex> lock( m_mutex );

			// make sure resource ( and its mips ) would fit the uplaod heap, if not then please make the upload heap bigger.
			assert( uSize < ( size_t)( m_pDataBegin - m_pDataEnd) );

			m_pDataCur = reinterpret_cast<UINT8*>(AlignUp( reinterpret_cast<SIZE_T>(m_pDataCur), uAlign ) );
			uSize = AlignUp( uSize, uAlign );

			// return null if we ran out of space in heap.
			if( (m_pDataCur >= m_pDataEnd) || (m_pDataCur + uSize >= m_pDataEnd ) )
			{
				return NULL;
			}

			pRet = m_pDataCur;
			m_pDataCur += uSize;
		}

		return pRet;
	}

	UINT8* UploadHeap::BeginSuballocate(SIZE_T uSize, UINT64 uAlign)
	{
		UINT8* pRes = NULL;

		for(;;)
		{
			pRes = Suballocate( uSize, uAlign );
			if( pRes != NULL )
			{
				break;
			}

			FlushAndFinish();
		}

		allocating.Inc();

		return pRes;
	}

	void UploadHeap::EndSuballocate()
	{
		allocating.Dec();
	}

	void UploadHeap::AddCopy(VkImage image, VkBufferImageCopy bufferImageCopy)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_copies.push_back({ image, bufferImageCopy });
	}

	void UploadHeap::AddPreBarrier(VkImageMemoryBarrier imageMemoryBarrier)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_toPreBarrier.push_back({ imageMemoryBarrier });
	}

	void UploadHeap::AddPostBarrier(VkImageMemoryBarrier imageMemoryBarrier)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_toPostBarrier.push_back({ imageMemoryBarrier });
	}

	void UploadHeap::Flush()
	{
		VkResult res;

		VkMappedMemoryRange range[1] = {};
		range[0].sType	= VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[0].memory	= m_deviceMemory;
		range[0].size	= m_pDataCur - m_pDataBegin;
		res = vkFlushMappedMemoryRanges( m_pDevice->GetDevice(), 1, range );
		assert( res == VK_SUCCESS );
	}

	//--------------------------------------------------------------------------------------
	//
	// FlushAndFinish
	//
	//--------------------------------------------------------------------------------------
	void UploadHeap::FlushAndFinish( bool bDoBarriers )
	{
		// Making sure that laready running thread is not flushing.
		flushing.Wait();

		// Begins a critical section, and make sure no allocation happens while a thread is inside it.
		flushing.Inc();

		// Wait for pending allocations to finish.
		allocating.Wait();

		std::unique_lock<std::mutex> lock( m_mutex );
		Flush();
		Trace( "flushing %i", m_copies.size() );

		// Apply per barrier in one go.
		if( m_toPreBarrier.size() > 0 )
		{
			vkCmdPipelineBarrier( GetCommandList(), VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
								  NULL, 0, NULL, (uint32_t)m_toPreBarrier.size(), m_toPreBarrier.data() );
			m_toPreBarrier.clear();
		}

		for ( COPY c : m_copies )
		{
			vkCmdCopyBufferToImage( GetCommandList(), GetResource(), c.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &c.m_bufferImageCopy );
		}
		m_copies.clear();

		// Apply post barrier in one go.
		if ( m_toPostBarrier.size() > 0 )
		{
			vkCmdPipelineBarrier( GetCommandList(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
								  NULL, 0, NULL, (uint32_t)m_toPostBarrier.size(), m_toPostBarrier.data() );
			m_toPostBarrier.clear();
		}

		// Close.
		VkResult res = vkEndCommandBuffer( m_commandBuffer );
		assert( res == VK_SUCCESS );

		// Submit.
		const VkCommandBuffer cmd_bufs[] = { m_commandBuffer };
		VkSubmitInfo submit_info;
		submit_info.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext					= NULL;
		submit_info.waitSemaphoreCount		= 0;
		submit_info.pWaitSemaphores			= NULL;
		submit_info.pWaitDstStageMask		= NULL;
		submit_info.commandBufferCount		= 1;
		submit_info.pCommandBuffers			= cmd_bufs;
		submit_info.signalSemaphoreCount	= 0;
		submit_info.pSignalSemaphores		= NULL;

		res = vkQueueSubmit( m_pDevice->GetGraphicsQueue(), 1, &submit_info, m_fence );
		assert( res == VK_SUCCESS );

		// Make sure that its has been proccessed by the GPU.
		res = vkWaitForFences( m_pDevice->GetDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX );
		assert( res == VK_SUCCESS );

		vkResetFences( m_pDevice->GetDevice(), 1, &m_fence );

		// Reset so that it can be reused.
		VkCommandBufferBeginInfo cmd_buf_info = {};
		cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		res = vkBeginCommandBuffer( m_commandBuffer, &cmd_buf_info );
		assert( res == VK_SUCCESS );

		m_pDataCur = m_pDataBegin;

		flushing.Dec();
	}
}