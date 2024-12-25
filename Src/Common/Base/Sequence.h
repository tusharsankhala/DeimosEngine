#pragma once

#include <json/json.h>
#include "Common/Misc/Camera.h"

using json = nlohmann::json;

class BenchmarkSequence
{
public:
	struct KeyFrame
	{
		float			m_time;
		int				m_camera;
		math::Vector4	m_from, m_to;
		std::string		m_screenShotName;
	};

	void ReadKeyframes(const json& jSequence, float timeStart, float timeEnd);
	float GetTimeStart() { return m_timeStart; }
	float GetTimeEnd() { return m_timeEnd; }

	//
	// Given a time return the time of the next keyframe, that way we know how long do we have to wait until
	// we apply the next keyframe.
	//
	float	GetNextKeyTime(float time);
	const KeyFrame GetNextKeyFrame(float time) const;

private:
	float					m_timeStart;
	float					m_timeEnd;

	std::vector<KeyFrame>	m_keyFrames;
};