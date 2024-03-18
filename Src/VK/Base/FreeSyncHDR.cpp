
#include "Common/stdafx.h"
#include <VK/Extensions/ExtFreeSyncHDR.h>
#include "FreeSyncHDR.h"

#include <vulkan/vulkan.h>

#include <unordered_map>


namespace Engine_VK
{
	static VkSurfaceFullScreenExclusiveWin32InfoEXT s_surfaceFullScreenExclusivewin32InfoEXT;
	static VkSurfaceFullScreenExclusiveInfoEXT		s_surfaceFullScreenExclusiveInfoEXT;
	static VkPhysicalDeviceSurfaceInfo2KHR			s_physicalDeviceSurfaceInfo2KHR;
	static VkDisplayNativeHdrSurfaceCapabilitiesAMD s_displayNativeHDRSurfaceCapabilitiesAMD;

	static VkHdrMetadataEXT							s_hdrMetadataExt;
	static VkSurfaceCapabilities2KHR				s_surfaceCapabilities2KHR;
	static VkSwapchainDisplayNativeHdrCreateInfoAMD	s_swapchainDisplayNativeHDRCreateInfoAMD;

	static VkDevice									s_device;
	static VkPhysicalDevice							s_physicalDevice;
	static VkSurfaceKHR								s_surface;

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
	// fsHdrEnumerateDisplayModes, enumerates availabe modes
	//
	//--------------------------------------------------------------------------------------
	void fsHdrEnumerateDisplayModes( std::vector<DisplayMode>* pModes,	bool includeFreesyncHDR, PresentationMode fullscreenMode, bool enableLocalDimming )
	{
		pModes->clear();
		availableDisplayModeSurfaceformats.clear();

		VkSurfaceFormatKHR surfaceFormat;
		surfaceFormat.format		= VK_FORMAT_R8G8B8A8_UNORM;
		surfaceFormat.colorSpace	= VK_COLORSPACE_SRGB_NONLINEAR_KHR;

		pModes->push_back( DISPLAYMODE_SDR );
		availableDisplayModeSurfaceformats[ DISPLAYMODE_SDR ] = surfaceFormat;

		if( ExtAreHDRExtensionsPresent() )
		{
			SetHDRStructs( s_surface );

			uint32_t formatCount;
			std::vector<VkSurfaceFormat2KHR> surfFormats;
			GetSurfaceFormats( &formatCount, &surfFormats );
		}
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
			VkResult res = vkAcquireFullScreenExclusiveModeEXT( s_device, swapchain );
			assert( res == VK_SUCCESS );
		}
		else
		{
			VkResult res = vkReleaseFullScreenExclusiveModeEXT( s_device, swapchain );
			assert(res == VK_SUCCESS);
		}
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
