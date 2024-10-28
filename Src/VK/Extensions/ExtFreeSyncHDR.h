#pragma once

#include "VK/Base/DeviceProperties.h"
#include "VK/Base/InstanceProperties.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

namespace Engine_VK
{
	extern PFN_vkGetDeviceProcAddr							g_vkGetDeviceProcAddr;

	// Functions for regular HDR ex: HDR10.
	extern PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR	g_vkGetPhysicalDeviceSurfaceCapabilities2KHR;
	extern PFN_vkGetPhysicalDeviceSurfaceFormats2KHR		g_vkGetPhysicalDeviceSurfaceFormats2KHR;
	extern PFN_vkSetHdrMetadataEXT							g_vkSetHdrMetadataEXT;

	// Functions for FSE required if trying to use Freesync HDR.
	extern PFN_vkAcquireFullScreenExclusiveModeEXT			g_vkAcquireFullScreenExclusiveModeEXT;
	extern PFN_vkReleaseFullScreenExclusiveModeEXT			g_vkReleaseFullScreenExclusiveModeEXT;

	// Functions for Freesync HDR.
	extern PFN_vkSetLocalDimmingAMD							g_vkSetLocalDimmingAMD;

	static bool s_isHdrInstanceExtensionPresent = false;
	static bool s_isHdrDeviceExtensionsPresent = false;
	static bool s_isFSEDeviceExtensionsPresent = false;
	static bool s_isFSHDRDeviceExtensionsPresent = false;


	void ExtCheckHDRInstanceExtensions( InstanceProperties* pIP );
	void ExtCheckHDRDeviceExtensions( DeviceProperties* pDP );
	void ExtCheckFSEDeviceExtensions( DeviceProperties* pDP );
	void ExtCheckFreeSyncHDRDeviceExtensions( DeviceProperties* pDP );

    void ExtGetHDRFSEFreeSyncHDRProcAddresses( VkInstance instance, VkDevice device );

    bool ExtAreHDRExtensionsPresent();
    bool ExtAreFSEExtensionsPresent();
    bool ExtAreFreeSyncHDRExtensionsPresent();
}
