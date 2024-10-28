// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <windowsx.h>

// C RunTime Header Files
#include <malloc.h>
#include <map>
#include <vector>
#include <mutex>
#include <fstream>

#include "vulkan/vulkan.h"

// Pull in math library
#include <vectormath/vectormath.hpp>

// TODO: reference additional headers your program requires here
#include "VK/Base/Imgui.h"
#include "Common/Base/ImGuiHelper.h"
#include "VK/Base/Device.h"
#include "VK/Base/Helper.h"
#include "VK/Base/Texture.h"
#include "VK/Base/FrameworkWindows.h"
#include "VK/Base/FreeSyncHDR.h"
#include "VK/Base/SwapChain.h"
#include "VK/Base/UploadHeap.h"
#include "VK/Base/GPUTimeStamps.h"
#include "VK/Base/CommandListRing.h"
#include "VK/Base/StaticBufferPool.h"
#include "VK/Base/DynamicBufferRing.h"
#include "VK/Base/ResourceViewHeaps.h"

#include "VK/Base/ShaderCompilerHelper.h"


#include "Common/Misc/Misc.h"
#include "Common/Misc/Camera.h"




using namespace Engine_VK;
