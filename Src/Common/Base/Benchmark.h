#pragma once

#include <json/json.h>
#include "Common/Misc/Camera.h"
#include "Common/GLTF/GltfCommon.h"

using json = nlohmann::json;

class GLTFCommon;

struct TimeStamp
{
	std::string m_label;
	float		m_microseconds;
};

void BenchmarkConfig(const json& benchmark, int cameraId, GLTFCommon* pGltfLoader, const std::string& deviceName = "not set", const std::string& driverVersion = "not set");
float BenchmarkLoop(const std::vector<TimeStamp>& timeStamps, Camera* pCam, std::string& outScreenShotName);