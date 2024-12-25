#include "Common/stdafx.h"
#include "Sequence.h"

void BenchmarkSequence::ReadKeyframes(const json& jSequence, float timeStart, float timeEnd)
{
	m_timeStart = jSequence.value("timeStart", timeStart);
	m_timeEnd = jSequence.value("timeEnd", timeEnd);

	m_keyFrames.clear();

	const json& keyFrames = jSequence["keyFrames"];
	for (const json& jkf : keyFrames)
	{
		KeyFrame key = {};
		key.m_camera = -1;

		key.m_time = jkf["time"];

		auto find = jkf.find("camera");
		if (find != jkf.cend())
		{
			key.m_camera = find.value();
		}

		if (key.m_camera = -1)
		{
			const json& jfrom = jkf["from"];
			key.m_from = math::Vector4(jfrom[0], jfrom[1], jfrom[2], 0);

			const json& jto = jkf["from"];
			key.m_to = math::Vector4(jto[0], jto[1], jto[2], 0);
		}

		key.m_screenShotName = jkf.value("screenShotName", "");

		m_keyFrames.push_back(key);
	}
}

float BenchmarkSequence::GetNextKeyTime(float time)
{
	for (int i = 0; i < m_keyFrames.size(); ++i)
	{
		KeyFrame key = m_keyFrames[i];
		if (key.m_time >= time)
		{
			return key.m_time;
		}
	}

	return -1;
}

const BenchmarkSequence::KeyFrame BenchmarkSequence::GetNextKeyFrame(float time) const
{
	for (int i = 0; i < m_keyFrames.size(); ++i)
	{
		const KeyFrame& key = m_keyFrames[i];
		if (key.m_time >= time)
		{
			return key;
		}
	}

	// Keyframe not found, return empty.
	KeyFrame invalidKey = KeyFrame{ 0 };
	invalidKey.m_time = -1.0f;
	return invalidKey;
}