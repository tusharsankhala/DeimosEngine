#pragma once

#include <vulkan/vulkan.h>
#include "Common/GLTF/GltfHelpers.h"

namespace Engine_VK
{
	VkFormat GetFormat(const std::string& str, int id);
	uint32_t SizeOfFormat(VkFormat format);
}