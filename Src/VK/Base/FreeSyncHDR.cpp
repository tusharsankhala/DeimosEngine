
#include "Common/stdafx.h"
#include "VK/Extensions/ExtFreeSyncHDR.h"
#include "FreeSyncHDR.h"
#include "Common/Misc/Misc.h"
#include "Common/Misc/Error.h"
#include <vulkan/vulkan.h>

#include <unordered_map>


namespace Engine_VK
{
	static VkSurfaceFullScreenExclusiveWin32InfoEXT s_surfaceFullScreenExclusiveWin32InfoEXT;
	static VkSurfaceFullScreenExclusiveInfoEXT		s_surfaceFullScreenExclusiveInfoEXT;
	static VkPhysicalDeviceSurfaceInfo2KHR			s_physicalDeviceSurfaceInfo2KHR;
	static VkDisplayNativeHdrSurfaceCapabilitiesAMD s_displayNativeHDRSurfaceCapabilitiesAMD;

	static VkHdrMetadataEXT							s_hdrMetadataExt;
	static VkSurfaceCapabilities2KHR				s_surfaceCapabilities2KHR;
	static VkSwapchainDisplayNativeHdrCreateInfoAMD	s_swapchainDisplayNativeHDRCreateInfoAMD;

	static VkDevice									s_device;
	static VkPhysicalDevice							s_physicalDevice;
	static VkSurfaceKHR								s_surface;
	static HWND										s_hWnd = NULL;

	static std::unordered_map<DisplayMode, VkSurfaceFormatKHR> availableDisplayModeSurfaceformats;

	bool s_windowHDRToggle = false;

	void SetHDRStructs( VkSurfaceKHR surface )
	{
		s_physicalDeviceSurfaceInfo2KHR.sType	= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
		s_physicalDeviceSurfaceInfo2KHR.pNext	= nullptr;
		s_physicalDeviceSurfaceInfo2KHR.surface	= surface;

		s_hdrMetadataExt.sType					= VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
		s_hdrMetadataExt.pNext					= nullptr;
	}

	void SetFSEStructures( HWND hWnd, PresentationMode fullscreenMode )
	{
		// Required final chaining order.

		s_physicalDeviceSurfaceInfo2KHR.pNext		= &s_surfaceFullScreenExclusiveInfoEXT;
		s_surfaceFullScreenExclusiveInfoEXT.pNext	= &s_surfaceFullScreenExclusiveWin32InfoEXT;

		s_surfaceFullScreenExclusiveInfoEXT.sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT;
		if( fullscreenMode == PRESENTATIONMODE_WINDOWED )
			s_surfaceFullScreenExclusiveInfoEXT.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_DISALLOWED_EXT;
		else if ( fullscreenMode == PRESENTATIONMODE_BORDERLESS_FULLSCREEN )
			s_surfaceFullScreenExclusiveInfoEXT.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_ALLOWED_EXT;
		else if ( fullscreenMode == PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN )
			s_surfaceFullScreenExclusiveInfoEXT.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;

		s_surfaceFullScreenExclusiveWin32InfoEXT.sType		= VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT;
		s_surfaceFullScreenExclusiveWin32InfoEXT.pNext		= nullptr;
		s_surfaceFullScreenExclusiveWin32InfoEXT.hmonitor	= MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
	}

	void SetFreesyncHDRStructures()
	{
		// Required final chaning order.

		s_surfaceCapabilities2KHR.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR;
		s_surfaceCapabilities2KHR.pNext = &s_displayNativeHDRSurfaceCapabilitiesAMD;

		// This will cause validation error and complain as invalid structure attached.
		// But we are hijacking hdr metadata struct and attaching it here to fill it with monitor primaries
		// When getting surface capabilities.
		s_displayNativeHDRSurfaceCapabilitiesAMD.sType = VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD;
		s_displayNativeHDRSurfaceCapabilitiesAMD.pNext = &s_hdrMetadataExt;
	}

	void SetSwapchainFreesyncHDRStructures( bool enableLocalDimming )
	{
		// Required final chaning order
		// VkSurfaceFullScreenExclusiveInfoEXT -> VkSurfaceFullScreenExclusiveWin32InfoEXT -> VkSwapchainDisplayNativeHdrCreateInfoAMD
		s_surfaceFullScreenExclusiveWin32InfoEXT.pNext = &s_swapchainDisplayNativeHDRCreateInfoAMD;

		s_swapchainDisplayNativeHDRCreateInfoAMD.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD;
		s_swapchainDisplayNativeHDRCreateInfoAMD.pNext = nullptr;
		// Enable local dimming if supported
		// Must requry FS HDR display capabilities
		// After value is set through swapchain creation when attached to swapchain pnext.
		s_swapchainDisplayNativeHDRCreateInfoAMD.localDimmingEnable = s_displayNativeHDRSurfaceCapabilitiesAMD.localDimmingSupport && enableLocalDimming;
	}

	void GetSurfaceFormats( uint32_t* pFormatCount, std::vector<VkSurfaceFormat2KHR>* surfFormats )
	{
		// Get the list of Formats.
		//
		VkResult res = g_vkGetPhysicalDeviceSurfaceFormats2KHR( s_physicalDevice, &s_physicalDeviceSurfaceInfo2KHR, pFormatCount, NULL );
		assert( res == VK_SUCCESS );

		uint32_t formatCount = *pFormatCount;
		surfFormats->resize( formatCount );
		for (UINT i = 0; i < formatCount; ++i)
		{
			(*surfFormats)[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
		}

		res = g_vkGetPhysicalDeviceSurfaceFormats2KHR( s_physicalDevice, &s_physicalDeviceSurfaceInfo2KHR, &formatCount, (*surfFormats).data() );
		assert( res == VK_SUCCESS );
	}

	//--------------------------------------------------------------------------------------
	//
	// fsHdrInit, if it returns false then fsHDR extensions are not present.
	//
	//--------------------------------------------------------------------------------------
	bool fsHdrInit(VkDevice device, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, HWND hWnd)
	{
		s_hWnd			= hWnd;
		s_device		= device;
		s_surface		= surface;
		s_physicalDevice = physicalDevice;

		return ExtAreFreeSyncHDRExtensionsPresent();
	}

	//--------------------------------------------------------------------------------------
	//
	// fsHdrEnumerateDisplayModes, enumerates availabe modes
	//
	//--------------------------------------------------------------------------------------
	bool fsHdrEnumerateDisplayModes( std::vector<DisplayMode>* pModes,	bool includeFreesyncHDR, PresentationMode fullscreenMode, bool enableLocalDimming )
	{
		pModes->clear();
		availableDisplayModeSurfaceformats.clear();

		VkSurfaceFormatKHR surfaceFormat;
		surfaceFormat.format		= VK_FORMAT_R8G8B8A8_UNORM;
		surfaceFormat.colorSpace	= VK_COLORSPACE_SRGB_NONLINEAR_KHR;

		pModes->push_back( DISPLAYMODE_SDR );
		availableDisplayModeSurfaceformats[ DISPLAYMODE_SDR ] = surfaceFormat;

		if (ExtAreHDRExtensionsPresent())
		{
			SetHDRStructs(s_surface);

			uint32_t formatCount;
			std::vector<VkSurfaceFormat2KHR> surfFormats;
			GetSurfaceFormats(&formatCount, &surfFormats);

			for (uint32_t i = 0; i < formatCount; ++i)
			{
				if ((surfFormats[i].surfaceFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 &&
					surfFormats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
					||
					(surfFormats[i].surfaceFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 &&
						surfFormats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT))
				{
					// If surface formats have HDR10 format even before fullscreen surface is attached, it can only mean windows hdr toggle is on.
					s_windowHDRToggle = true;
					break;
				}
			}
		}
		else
		{
			s_physicalDeviceSurfaceInfo2KHR.sType	= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
			s_physicalDeviceSurfaceInfo2KHR.pNext	= nullptr;
			s_physicalDeviceSurfaceInfo2KHR.surface = s_surface;
		}

		if( ExtAreFSEExtensionsPresent () )
		{
			SetFSEStructures( s_hWnd, fullscreenMode );
		}

		if ( includeFreesyncHDR && ExtAreFreeSyncHDRExtensionsPresent() )
		{
			SetFreesyncHDRStructures();

			// Calling the capabilities here to query for local dimming support.
			fsHdrGetPhysicalDeviceSurfaceCapabilities2KHR( s_physicalDevice, s_surface, NULL );

			SetSwapchainFreesyncHDRStructures( enableLocalDimming );
		}

		uint32_t formatCount;
		std::vector<VkSurfaceFormat2KHR> surfFormats;
		GetSurfaceFormats( &formatCount, &surfFormats );

		for ( uint32_t i = 0; i < formatCount; ++i )
		{
			if ( surfFormats[i].surfaceFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 &&
				 surfFormats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_DISPLAY_NATIVE_AMD )
			{
				pModes->push_back( DISPLAYMODE_FSHDR_Gamma22 );
				availableDisplayModeSurfaceformats[ DISPLAYMODE_FSHDR_Gamma22 ] = surfFormats[i].surfaceFormat;
			}
			else if ( surfFormats[i].surfaceFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT &&
					  surfFormats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_DISPLAY_NATIVE_AMD )
			{
				pModes->push_back(DISPLAYMODE_FSHDR_Gamma22);
				availableDisplayModeSurfaceformats[DISPLAYMODE_FSHDR_Gamma22] = surfFormats[i].surfaceFormat;
			}
		}
	
		for (uint32_t i = 0; i < formatCount; ++i)
		{
			if (( surfFormats[i].surfaceFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 &&
				  surfFormats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT )
				||
				( surfFormats[i].surfaceFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 &&
				  surfFormats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT))
			{
				if (availableDisplayModeSurfaceformats.find(DISPLAYMODE_HDR10_2048) == availableDisplayModeSurfaceformats.end())
				{
					pModes->push_back(DISPLAYMODE_HDR10_2048);
					availableDisplayModeSurfaceformats[DISPLAYMODE_HDR10_2048] = surfFormats[i].surfaceFormat;
				}
			}
			else if ( surfFormats[i].surfaceFormat.format == VK_FORMAT_R16G16B16A16_SFLOAT &&
				surfFormats[i].surfaceFormat.colorSpace == VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT )
			{
				pModes->push_back( DISPLAYMODE_HDR10_SCRGB );
				availableDisplayModeSurfaceformats[ DISPLAYMODE_HDR10_SCRGB ] = surfFormats[i].surfaceFormat;
			}
		}

		return true;
	}

	//--------------------------------------------------------------------------------------
	//
	// fsHdrSetFullscreenState
	//
	//--------------------------------------------------------------------------------------
	void fsHdrSetFullscreenState( bool fullscreen, VkSwapchainKHR swapchain )
	{
		if( fullscreen )
		{
			VkResult res = g_vkAcquireFullScreenExclusiveModeEXT( s_device, swapchain );
			assert( res == VK_SUCCESS );
		}
		else
		{
			VkResult res = g_vkReleaseFullScreenExclusiveModeEXT( s_device, swapchain );
			assert(res == VK_SUCCESS);
		}
	}

	//--------------------------------------------------------------------------------------
	//
	// fsHdrGetDisplayInfo
	//
	//--------------------------------------------------------------------------------------
	const VkHdrMetadataEXT* fsHdrGetDisplayInfo()
	{
		return &s_hdrMetadataExt;
	}

	void fsHdrGetPhysicalDeviceSurfaceCapabilities2KHR( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfCapabilities )
	{
		assert( surface == s_surface );
		assert( physicalDevice == s_physicalDevice );

		VkResult res = g_vkGetPhysicalDeviceSurfaceCapabilities2KHR( s_physicalDevice, &s_physicalDeviceSurfaceInfo2KHR, &s_surfaceCapabilities2KHR );
		assert( res == VK_SUCCESS );

		if( pSurfCapabilities )
			*pSurfCapabilities = s_surfaceCapabilities2KHR.surfaceCapabilities;
	}

	// We are turning off this HDR path for now
	// Driver update to get this path working is coming soon.
	#define WINDOW_HRD_PATH 0
	const bool CheckIfWindowModeHdrOn()
	{
		#if WINDOW_HDR_PATH
			return s_windowHdrToggle;
		#else
			return false;
		#endif

	}
};
