#pragma once

#include "DeviceProperties.h"
#include "InstanceProperties.h"

#include <vulkan/vulkan_win32.h>

namespace Engine_VK
{
	enum PresentationMode
	{
		PRESENTATIONMODE_WINDOWED,
		PRESENTATIONMODE_BORDERLESS_FULLSCREEN,
		PRESENTATIONMODE_EXCLUSIVE_FULLSCREEN,
	};

	enum DisplayMode
	{
		DISPLAYMODE_SDR,
		DISPLAYMODE_FSHDR_Gamma22,
		DISPLAYMODE_FSHDR_SCRGB,
		DISPLAYMODE_HDR10_2048,
		DISPLAYMODE_HDR10_SCRGB,
	};

	// Only the swapchain should be using these functions.
	bool fsHdrInit(VkDevice device, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, HWND hWnd);
	bool fsHdrEnumerateDisplayModes( std::vector<DisplayMode>* pModes, bool includeFreesyncHDR, PresentationMode fullscreenMode, bool enableLocalDimming );
	const VkHdrMetadataEXT* fsHdrGetDisplayInfo();

	void fsHdrSetFullscreenState( bool fullscreen, VkSwapchainKHR swapchain );

	void fsHdrGetPhysicalDeviceSurfaceCapabilities2KHR( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfCapabilities );


	const bool CheckIfWindowModeHdrOn();
}