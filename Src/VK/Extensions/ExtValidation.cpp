#include "Common/stdafx.h"
#include <vulkan/vulkan_win32.h>
#include "VK/Base/Instance.h"
#include "VK/Base/InstanceProperties.h"
#include "VK/Base/DeviceProperties.h"
#include "ExtValidation.h"

namespace Engine_VK
{
	static PFN_vkCreateDebugReportCallbackEXT	g_vkCreateDebugReportCallbackEXT	= NULL;
	static PFN_vkDebugReportMessageEXT			g_vkDebugReportMessageEXT			= NULL;
	static PFN_vkDestroyDebugReportCallbackEXT	g_vkDestroyDebugReportCallbackEXT	= NULL;
	static VkDebugReportCallbackEXT				g_debugReportCallback				= NULL;

	static bool s_bCanUseDebugReport = false;

	static VKAPI_ATTR VkBool32 VKAPI_CALL MyDebugReportCallback(
		VkDebugReportFlagsEXT		flags,
		VkDebugReportObjectTypeEXT	objectType,
		uint64_t					object,
		size_t						location,
		int32_t						messageCode,
		const char*					pLayerPrefix,
		const char*					pMessage,
		void*						pUserData )
	{
		OutputDebugStringA( pMessage );
		OutputDebugStringA( "\n" );
		return VK_FALSE;
	}
	
	const VkValidationFeatureEnableEXT featuresRequested[] = { VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
															   VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
															   VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };

	VkValidationFeaturesEXT features = {};

	const char instanceExtensionName[] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
	const char instanceLayerName[] = "VK_LAYER_KHRONOS_validation";

	bool ExtDebugReportCheckInstanceExtensions( InstanceProperties* pIP, bool gpuValidation )
	{
		s_bCanUseDebugReport = pIP->AddInstanceLayerName( instanceLayerName ) && pIP->AddInstanceExtensionName( instanceExtensionName );
		if( s_bCanUseDebugReport && gpuValidation )
		{
			features.sType							= VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
			features.pNext							= pIP->GetNext();
			features.enabledValidationFeatureCount	= _countof( featuresRequested );
			features.pEnabledValidationFeatures		= featuresRequested;

			pIP->SetNewNext( &features );
		}

		return s_bCanUseDebugReport;
	}

	void ExtDebugReportGetProcAddresses( VkInstance instance )
	{
		if( s_bCanUseDebugReport )
		{
			g_vkCreateDebugReportCallbackEXT	= ( PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT");
			g_vkDebugReportMessageEXT			= (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr( instance, "vkDebugReportMessageEXT");
			g_vkDestroyDebugReportCallbackEXT	= (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT");

			assert(g_vkCreateDebugReportCallbackEXT);
			assert(g_vkDebugReportMessageEXT);
			assert(g_vkDestroyDebugReportCallbackEXT);
		}
	}

	void ExtDebugReportOnCreate(VkInstance instance)
	{
		if( g_vkCreateDebugReportCallbackEXT )
		{
			VkDebugReportCallbackCreateInfoEXT debugReportCallbackInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
			debugReportCallbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			debugReportCallbackInfo.pfnCallback = MyDebugReportCallback;
			VkResult res = g_vkCreateDebugReportCallbackEXT( instance, &debugReportCallbackInfo, nullptr, &g_debugReportCallback );
			assert( res == VK_SUCCESS ); 
		}
	}

	void ExtDebugReportOnDestroy(VkInstance instance)
	{
		// It should happed after destroying device, before destorying instance.
		if( g_debugReportCallback )
		{
			g_vkDestroyDebugReportCallbackEXT( instance, g_debugReportCallback, nullptr );
			g_debugReportCallback = nullptr;
		}
	}
}