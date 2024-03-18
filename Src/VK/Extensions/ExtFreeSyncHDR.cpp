#include "Common/stdafx.h"

#include "ExtFreeSyncHDR.h"
#include "Common/Misc/Misc.h"

namespace Engine_VK
{
	extern PFN_vkGetDeviceProcAddr							g_vkGetDeviceProcAddr = NULL;

	// Functions for regular HDR ex: HDR10.
	extern PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR	g_vkGetPhysicalDeviceSurfaceCapabilities2KHR = NULL;
	extern PFN_vkGetPhysicalDeviceSurfaceFormats2KHR		g_vkGetPhysicalDeviceSurfaceFormats2KHR = NULL;
	extern PFN_vkSetHdrMetadataEXT							g_vkSetHdrMetadataEXT = NULL;

	// Functions for FSE required if trying to use Freesync HDR.
	extern PFN_vkAcquireFullScreenExclusiveModeEXT			g_vkAcquireFullScreenExclusiveModeEXT = NULL;
	extern PFN_vkReleaseFullScreenExclusiveModeEXT			g_vkReleaseFullScreenExclusiveModeEXT = NULL;
	
	// Functions for Freesync HDR.
	extern PFN_vkSetLocalDimmingAMD							g_vkSetLocalDimmingAMD = NULL;

	static VkPhysicalDeviceSurfaceInfo2KHR					s_physicalDeviceSurfaceInfo2KHR;

	static VkSurfaceFullScreenExclusiveWin32InfoEXT			s_surfaceFullScreenExclusiveWin32InfoEXT;
	static VkSurfaceFullScreenExclusiveInfoEXT				s_surfaceFullScreenExclusiveInfoEXT;

	static VkDisplayNativeHdrSurfaceCapabilitiesAMD			s_displayNativeHdrSurfaceCapabilitiesAMD;

	static bool s_isHdrInstanceExtensionsPresent		= false;
	static bool s_isHdrDeviceExtensionsPresent			= false;
	static bool s_isFSEDeviceExtensionsPresent			= false;
	static bool s_isFSHDRDeviceExtensionsPresent		= false;

	void ExtCheckHDRInstanceExtensions( InstanceProperties* pIP )
	{
		s_isHdrInstanceExtensionsPresent = pIP->AddInstanceExtensionName( VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME );
	}

	void ExtCheckHDRDeviceExtensions( DeviceProperties* pDP )
	{
		s_isHdrDeviceExtensionsPresent = pDP->AddDeviceExtensionName( VK_EXT_HDR_METADATA_EXTENSION_NAME );
	}

	void ExtCheckFSEDeviceExtensions( DeviceProperties* pDP )
	{
		s_isFSEDeviceExtensionsPresent = pDP->AddDeviceExtensionName( VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME );
	}

	void ExtCheckFreeSyncHDRDeviceExtensions( DeviceProperties* pDP )
	{
		s_isFSHDRDeviceExtensionsPresent = pDP->AddDeviceExtensionName( VK_AMD_DISPLAY_NATIVE_HDR_EXTENSION_NAME );
	}


	void ExtGetHRDFSEFreesyncHDRProcAddresses(VkInstance instance, VkDevice device)
	{
		if( s_isHdrDeviceExtensionsPresent )
		{
			g_vkGetPhysicalDeviceSurfaceCapabilities2KHR = ( PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR )vkGetInstanceProcAddr( instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR" );
			assert( g_vkGetPhysicalDeviceSurfaceCapabilities2KHR );

			g_vkGetPhysicalDeviceSurfaceFormats2KHR = ( PFN_vkGetPhysicalDeviceSurfaceFormats2KHR )vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR" );
			assert( g_vkGetPhysicalDeviceSurfaceFormats2KHR );
		}

		g_vkGetDeviceProcAddr = ( PFN_vkGetDeviceProcAddr )vkGetInstanceProcAddr( instance, "vkGetdeviceProcAddr" );
		assert(g_vkGetDeviceProcAddr);

		if ( s_isHdrDeviceExtensionsPresent )
		{
			g_vkSetHdrMetadataEXT = ( PFN_vkSetHdrMetadataEXT )g_vkGetDeviceProcAddr( device, "vkSetHdrMetadataEXT" );
			assert( g_vkSetHdrMetadataEXT );
		}

		if ( s_isFSEDeviceExtensionsPresent )
		{
			g_vkAcquireFullScreenExclusiveModeEXT = ( PFN_vkAcquireFullScreenExclusiveModeEXT ) vkGetDeviceProcAddr( device, "vkAcquireFullScreenExclusiveModeEXT" );
			assert( g_vkAcquireFullScreenExclusiveModeEXT );

			g_vkReleaseFullScreenExclusiveModeEXT = ( PFN_vkReleaseFullScreenExclusiveModeEXT ) vkGetDeviceProcAddr( device, "vkReleaseFullScreenExclusiveModeEXT");
			assert( g_vkReleaseFullScreenExclusiveModeEXT );
		}

		if ( s_isFSHDRDeviceExtensionsPresent )
		{
			g_vkSetLocalDimmingAMD = ( PFN_vkSetLocalDimmingAMD )g_vkGetDeviceProcAddr(device, "vkSetLocalDimmingAMD");
			assert( g_vkSetLocalDimmingAMD );
		}

	}

	bool ExtAreHDRExtensionsPresent()
	{
		return s_isHdrInstanceExtensionsPresent && s_isHdrDeviceExtensionsPresent;
	}

	bool ExtAreFSEEExtensionsPresent()
	{
		return s_isFSEDeviceExtensionsPresent;
	}

	bool ExtAreFreeSyncHDRExtensionsPresent()
	{
		return s_isHdrInstanceExtensionsPresent && 
			   s_isHdrDeviceExtensionsPresent   &&
			   s_isFSEDeviceExtensionsPresent	&&
			   s_isFSHDRDeviceExtensionsPresent;
	}
}