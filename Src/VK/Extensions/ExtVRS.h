#pragma once

#include "VK/Base/DeviceProperties.h"
#include "VK/Base/InstanceProperties.h"

namespace Engine_VK
{
	void ExtVRSCheckExtensions( DeviceProperties* pDP, bool& Tier1Supported, bool& Tier2Supported );
}