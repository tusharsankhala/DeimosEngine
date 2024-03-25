#pragma once

#include "VK/Base/DeviceProperties.h"
#include "VK/Base/InstanceProperties.h"

namespace Engine_VK
{
	void ExtRTCheckExtensions( DeviceProperties* pDP, bool& RT10Supported, bool& RT11Supported );
}