#include "Common/stdafx.h"
#include "VK/Base/Device.h"
#include "VK/Base/GPUTimestamps.h"

namespace Engine_VK
{
	void GPUTimestamps::OnCreate(Device* pDevice, uint32_t numberOfBackBuffers)
	{
		m_pDevice = pDevice;
		m_numberOfBackBuffers = numberOfBackBuffers;
		m_frame = 0;

		const VkQueryPoolCreateInfo queryPoolCreateInfo = 
		{
			VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,			// VkStructureType					sType
			NULL,												// const void*						pNext
			(VkQueryPoolCreateFlags)0,							// VkQueryPoolCreateFlags			flags
			VK_QUERY_TYPE_TIMESTAMP,							// VkQueryType						queryType
			maxValuesPerFrame * numberOfBackBuffers,			// deUint32							entryCount
			0,													// VkQueryPipelineStatisticFlags	pipelineStatistics
		};

		VkResult res = vkCreateQueryPool( pDevice->GetDevice(), &queryPoolCreateInfo, NULL, &m_queryPool );
	}

	void GPUTimestamps::OnDestroy()
	{
		vkDestroyQueryPool( m_pDevice->GetDevice(), m_queryPool, nullptr );

		for( uint32_t i = 0; i < m_numberOfBackBuffers; ++i )
		{
			m_labels[i].clear();
		}
	}

	void GPUTimestamps::GetTimeStamp( VkCommandBuffer cmd_buf, const char* label )
	{
		uint32_t measurements = (uint32_t)m_labels[m_frame].size();
		uint32_t offset = m_frame * maxValuesPerFrame + measurements;

		vkCmdWriteTimestamp( cmd_buf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_queryPool, offset );

		m_labels[ m_frame ].push_back( label );
	}

	void GPUTimestamps::GetTimeStampUser( TimeStamp ts )
	{
		m_cpuTimeStamps[ m_frame ].push_back( ts );
	}

	void GPUTimestamps::OnBeginFrame( VkCommandBuffer cmd_buf, std::vector<TimeStamp>* pTimeStamps )
	{
		std::vector<TimeStamp>& cpuTimeStamps = m_cpuTimeStamps[ m_frame ];
		std::vector<std::string>& gpuLabels = m_labels[ m_frame ];

		pTimeStamps->clear();
		pTimeStamps->reserve( cpuTimeStamps.size() + gpuLabels.size() );

		// Copy CPU timestamps.
		//
		for( uint32_t i=0; i < cpuTimeStamps.size(); ++i )
		{
			pTimeStamps->push_back( cpuTimeStamps[i] );
		}

		// Copy GPU timestamps.
		//
		uint32_t offset = m_frame - maxValuesPerFrame;
		uint32_t measurements = (uint32_t)gpuLabels.size();
		if( measurements > 0 )
		{
			// Timestamp period is the number of nanaseconds per timestamp value increment.
			double microsecondPerTick = (1e-3f * m_pDevice->GetPhysicalDeviceProperties().limits.timestampPeriod );
			{
				UINT64 timingInTicks[256] = {};
				VkResult res = vkGetQueryPoolResults( m_pDevice->GetDevice(), m_queryPool, offset, measurements, measurements * sizeof(UINT64), &timingInTicks,
													  sizeof(UINT64), VK_QUERY_RESULT_64_BIT );
				if( res == VK_SUCCESS )
				{
					for( uint32_t i = 1; i < measurements; ++i )
					{
						TimeStamp ts = { m_labels[ m_frame ][i], float( microsecondPerTick * (double)(timingInTicks[ i ] - timingInTicks[ i - 1 ])) };
						pTimeStamps->push_back( ts );
					}
				}
			
				// Compute total.
				TimeStamp ts = { "Total GPU Time", float(microsecondPerTick * (double)(timingInTicks[ measurements - 1 ] - timingInTicks[0])) };
			}
		}
		else
		{
			pTimeStamps->push_back({ "GPU counters are invalid", 0.0f } );
		}

		vkCmdResetQueryPool( cmd_buf, m_queryPool, offset, maxValuesPerFrame );
		
		// We always need to clear this one
		cpuTimeStamps.clear();
		gpuLabels.clear();

		GetTimeStamp( cmd_buf, "Begin Frame" );
	}

	void GPUTimestamps::OnEndFrame()
	{
		m_frame = ( m_frame + 1 ) % m_numberOfBackBuffers;
	}
}


