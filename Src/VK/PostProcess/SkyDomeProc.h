#pragma once

#include "PostProcPS.h"

#include <vectormath/vectormath.hpp>

namespace Engine_VK
{
	// This renders a procedural sky, see the SkyDomeProc.glsl for more references and credits.

	class SkyDomeProc
	{
	public:

		struct Constants
		{
			math::Matrix4		invViewProj;
			math::Vector4		vSunDirection;
			float				rayleigh;
			float				turbidity;
			float				mieCoefficient;
			float				luminance;
			float				mieDirectionalG;
		};

		void Draw(VkCommandBuffer cmd_buf, SkyDomeProc::Constants constants);

	private:

		Device*					m_pDevice;
		ResourceViewHeaps*		m_pResourceViewHeaps;

		VkDescriptorSet			m_descriptorSet;
		VkDescriptorSetLayout	m_descriptorLayout;

		PostProcPS				m_skydome;

		DynamicBufferRing*		m_pDynamicBufferRing = NULL;
	};
}
