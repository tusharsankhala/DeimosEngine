#include "Common/stdafx.h"
#include "VK/Extensions/ExtVRS.h"
#include "Common/Misc/Misc.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>


namespace Engine_VK
{
	static VkPhysicalDeviceFragmentShadingRateFeaturesKHR VRSQueryFeature = {};

	void ExtVRSCheckExtensions( DeviceProperties* pDP, bool& Tier1Supported, bool& Tier2Supported )
	{
		if( pDP->AddDeviceExtensionName( VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME ) )
		{
			pDP->AddDeviceExtensionName( VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME );

			VRSQueryFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_FEATURES_KHR;
			VkPhysicalDeviceFeatures2 features = {};
			features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features.pNext = &VRSQueryFeature;
			vkGetPhysicalDeviceFeatures2( pDP->GetPhysicalDevice(), &features );

			Tier1Supported = VRSQueryFeature.pipelineFragmentShadingRate;
			Tier2Supported = VRSQueryFeature.attachmentFragmentShadingRate && VRSQueryFeature.primitiveFragmentShadingRate;
		}
	}
}