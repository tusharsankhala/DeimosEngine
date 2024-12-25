#pragma once

#include <vector>
#include <string>
#include "Common/Base/Benchmark.h"

struct TimeStamp;

namespace Engine_VK
{
	// This class helps insert queries in the command buffer and readback the results.
	// The tricky part in fact is reading back the results without stalling the GPU. 
	// For that it splits the readback heap in <numberOfBackBuffers> pieces and it reads 
	// from the last used chuck.

	class GPUTimestamps
	{
	public:
		void OnCreate( Device* pDevice, uint32_t numberOfBackBuffers );
		void OnDestroy();

		void GetTimeStamp( VkCommandBuffer cmd_buf, const char* label );
		void GetTimeStampUser(TimeStamp ts );
		void OnBeginFrame( VkCommandBuffer cmd_buf, std::vector<TimeStamp>* pTimeStamps );
		void OnEndFrame();

	private:
		Device*			m_pDevice;

		const uint32_t	maxValuesPerFrame		= 128;

		VkQueryPool		m_queryPool;

		uint32_t		m_frame					= 0;
		uint32_t		m_numberOfBackBuffers	= 0;

		std::vector<std::string> m_labels[3];
		std::vector<TimeStamp> m_cpuTimeStamps[3];
	};
}
