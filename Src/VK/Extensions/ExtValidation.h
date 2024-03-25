#pragma once

#include "VK/Base/InstanceProperties.h"
#include "VK/Base/DeviceProperties.h"

namespace Engine_VK
{
	bool ExtDebugReportCheckInstanceExtensions( InstanceProperties* pIP, bool gpuValidation );
	void ExtDebugReportGetProcAddresses( VkInstance instance );

	void ExtDebugReportOnCreate( VkInstance instance );
	void ExtDebugReportOnDestroy( VkInstance instance );
}