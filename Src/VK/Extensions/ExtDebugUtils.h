#pragma once

#include <vulkan/vulkan.h>
#include "VK/Base/InstanceProperties.h"

namespace Engine_VK
{
	// Helper functions to use the debug markers
	void ExtDebugUtilsGetProcAddresses( VkDevice device );
	bool ExtDebugUtilsCheckInstanceExtensions( InstanceProperties* pDp );
	void SetResourceName( VkDevice device, VkObjectType objectType, uint64_t handle, const char* name );
	void SetPerfMarkerBegin( VkCommandBuffer cmd_buf, const char* name );
	void SetPerfMarkerEnd( VkCommandBuffer cmd_buf );
}