// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <SDKDDKVer.h>

#define NOMINMAX    
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <KnownFolders.h>
#include <shlobj.h>
#include <cstdarg>

// C RunTime Header Files
#include <malloc.h>
#include <tchar.h>
#include <cassert>

// GFX API 
#include <vulkan/vulkan.h>

// math API
#include <DirectXMath.h>
using namespace DirectX;

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <limits>
#include <algorithm>
#include <mutex>

#include "Common/Base/ImGuiHelper.h"
#include "VK/Base/Device.h"
#include "VK/Base/Helper.h"
#include "VK/Base/Texture.h"
#include "VK/Base/ResourceViewHeaps.h"
#include "VK/Base/SwapChain.h"
#include "VK/Base/CommandListRing.h"
#include "VK/Base/StaticBufferPool.h"
#include "VK/Base/DynamicBufferRing.h"
#include "VK/Base/GBuffer.h"
#include "VK/Base/GPUTimestamps.h"
#include "VK/Base/UploadHeap.h"

using namespace Engine_VK;

//#include <Shellapi.h> 

// TODO: reference additional headers your program requires here
