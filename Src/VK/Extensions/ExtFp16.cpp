#include "Common/stdafx.h"

#include "ExtFp16.h"
#include "Common/Misc/Misc.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

namespace Engine_VK
{
	static VkPhysicalDeviceFloat16Int8FeaturesKHR	FP16Features					= {};
	static VkPhysicalDevice16BitStorageFeatures		Storage16BitFeatures			= {};

	bool ExtFp16CheckExtensions( DeviceProperties* pDP )
	{
		std::vector<const char*> required_extensions_names = { VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME };

		bool bFp16Enabled = true;
		for (auto& ext : required_extensions_names)
		{
			if (pDP->AddDeviceExtensionName(ext) == false)
			{
				Trace( format( "FP16 disabled, missing extensions: %s\n", ext ));
				bFp16Enabled = false;
			}
		}

		if (bFp16Enabled)
		{
			// Query 16 bit storage.
			Storage16BitFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;

			VkPhysicalDeviceFeatures2 features = {};
			features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			features.pNext = &Storage16BitFeatures;
			vkGetPhysicalDeviceFeatures2( pDP->GetPhysicalDevice(), &features );

			bFp16Enabled = bFp16Enabled && Storage16BitFeatures.storageBuffer16BitAccess;

			// Query 16 bit ops.
			FP16Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;

			features.pNext = &FP16Features;
			vkGetPhysicalDeviceFeatures2( pDP->GetPhysicalDevice(), &features );
			
			bFp16Enabled = bFp16Enabled && FP16Features.shaderFloat16;
		}

		if (bFp16Enabled)
		{
			// Query 16 bit storage.
			Storage16BitFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES;
			Storage16BitFeatures.pNext = pDP->GetNext();


			// Query 16 bit ops.
			FP16Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR;
			FP16Features.pNext = &Storage16BitFeatures;
			
			pDP->SetNewNext( &FP16Features );
		}

		return bFp16Enabled;
	}
}
