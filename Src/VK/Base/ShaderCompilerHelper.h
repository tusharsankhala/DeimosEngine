#pragma once

#include "VK/Base/Device.h"
#include "Common/ShaderCompiler/ShaderCompiler.h"
#include "Common/ShaderCompiler/DXCHelper.h"

class Sync;

namespace Engine_VK
{
	enum ShaderSourceType
	{
		SST_HLSL,
		SST_GLSL,
	};

	void CreateShaderCache();
	void DestroyShaderCache(Device* pdevice);

	// Does as the function name says and uses a cache
	VkResult VKCompileFromString(VkDevice device, ShaderSourceType sourceType, const VkShaderStageFlagBits shader_type, const char* pShaderCode,
								 const char* pShaderEntryPoint, const char* pExtraParams, const DefineList* pDefines, VkPipelineShaderStageCreateInfo* pShader);

	VkResult VKCompileFromFile(VkDevice device, const VkShaderStageFlagBits shader_type, const char* pFilename, const char* pShaderEntryPoint, const char* pExtraParams,
							   const DefineList* pDefines, VkPipelineShaderStageCreateInfo* pShader);
}