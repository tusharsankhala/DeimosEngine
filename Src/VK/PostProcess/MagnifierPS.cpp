#include "Common/stdafx.h"
#include "VK/PostProcess/MagnifierPS.h"

#include "VK/Base/Device.h"
#include "VK/Base/DynamicBufferRing.h"
#include "VK/Extensions/ExtDebugUtils.h"
#include "VK/Base/SwapChain.h"
#include "Common/Misc/Misc.h"

#include <cassert>
#include <fstream>
#include <sstream>