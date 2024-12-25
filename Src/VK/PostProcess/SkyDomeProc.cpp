#include "Common/stdafx.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/PostProcess/SkyDomeProc.h"

namespace Engine_VK
{
	void SkyDomeProc::Draw(VkCommandBuffer cmd_buf, SkyDomeProc::Constants constants)
	{
		SetPerfMarkerBegin(cmd_buf, "Skydome Proc");

		SkyDomeProc::Constants* cbPerDraw;
		VkDescriptorBufferInfo	constantBuffer;
		m_pDynamicBufferRing->AllocConstantBuffer(sizeof(SkyDomeProc::Constants), (void**)&cbPerDraw, &constantBuffer);
		*cbPerDraw = constants;

		m_skydome.Draw(cmd_buf, &constantBuffer, m_descriptorSet);

		SetPerfMarkerEnd(cmd_buf);
	}
}