#include "Common/stdafx.h"
#include "Benchmark.h"
#include "Common/Base/Sequence.h"

struct Benchmark
{
	int					warmUpFrames;
	int					runningTimeWhenNoAnimation;
	FILE*				f = NULL;
	float				timeStep;
	float				time;
	float				timeStart, timeEnd;
	int					frame;
	bool				exitWhenTimeEnds;
	int					cameraId;

	GLTFCommon*			m_pGLFTLoader;
	
	BenchmarkSequence	m_sequence;

	bool				m_animationFound	= false;
	bool				m_saveHeaders		= true;
	float				m_nextTime			= 0;
};

static Benchmark bm;

static void SaveTimeStamps(float time, const std::vector<TimeStamp>& timeStamps)
{
	if (bm.m_saveHeaders)
	{
		// Save headers.
		fprintf(bm.f, "time");
		for (uint32_t i = 0; i < timeStamps.size(); ++i)
		{
			fprintf(bm.f, ", %s", timeStamps[i].m_label.c_str());
		}

		fprintf(bm.f, "\n");
		time = 0.0f;

		bm.m_saveHeaders = false;
	}

	// Save timings.
	fprintf(bm.f, "%f", time);
	for (uint32_t i = 0; i < timeStamps.size(); ++i)
	{
		fprintf(bm.f, ", %f", (float)timeStamps[i].m_microseconds);
	}

	fprintf(bm.f, "\n");
}

void BenchmarkConfig(const json& benchmark, int cameraId, GLTFCommon* pGltfLoader, const std::string& deviceName, const std::string& driverVersion)
{
	if (benchmark.is_null())
	{
		Trace("Benchmark section not found in the json, the scene needs a benchmark section for this to work \n");
		exit(0);
	}

	bm.f = NULL;
	bm.frame = 0;

	// The number of frames to run before the benchmark starts.
	bm.warmUpFrames = benchmark.value("warmUpFrames", 200);
	bm.exitWhenTimeEnds = benchmark.value("exitWhenTimeEnds", true);

	// Get filename and open it.
	std::string resultsFilename = benchmark.value("resultsFilename", "res.csv");
	bm.m_saveHeaders = true;
	if (fopen_s(&bm.f, resultsFilename.c_str(), "w") != 0)
	{
		Trace(format("The file %s cannot be opened\n", resultsFilename.c_str()));
		exit(0);
	}

	fprintf(bm.f, "#deviceName %s\n", deviceName.c_str());
	fprintf(bm.f, "#deverVersion %s\n", driverVersion.c_str());

	bm.timeStep = benchmark.value("timeStep", 1.0f);

	// Set default timeStart/timeEnd.
	bm.timeStart = 0;
	bm.timeEnd = 0;
	if ((pGltfLoader != NULL) && (pGltfLoader->m_animations.size() > 0))
	{
		// if there is an animation take the endTime from the animation.
		bm.timeEnd = pGltfLoader->m_animations[0].m_duration;
	}

	// override those values if set.
	bm.timeStart = benchmark.value("timeStart", bm.timeStart);
	bm.timeEnd = benchmark.value("timeEnd", bm.timeEnd);
	bm.time = bm.timeStart;

	// Sets the camera and its animation.
	//
	bm.m_animationFound = false;
	bm.cameraId = cameraId;
	if ((pGltfLoader == NULL) || (cameraId == -1))
	{
		if (benchmark.find("sequence") != benchmark.end())
		{
			// Camera will use the sequence.
			const json& sequence = benchmark["sequence"];
			bm.m_sequence.ReadKeyframes(sequence, bm.timeStart, bm.timeEnd);
			bm.timeStart = bm.m_sequence.GetTimeStart();
			bm.timeEnd = bm.m_sequence.GetTimeEnd();
			bm.m_animationFound = true;
			bm.cameraId = -1;
		}
		else
		{
			// will use no sequence, will use the default static camera
		}
	}
	else
	{
		// a camera from the GLTF will be used.
		// check such a camera exist, itherwise show an error and quit.
		Camera cam;
		if (pGltfLoader->GetCamera(cameraId, &cam) == false)
		{
			Trace(format("The cameraId %i doesn't exists in the GLTF\n", cameraId));
			exit(0);
		}

		bm.m_animationFound = true;
	}

	bm.m_nextTime		= 0;
	bm.m_pGLFTLoader	= pGltfLoader;
}

float BenchmarkLoop(const std::vector<TimeStamp>& timeStamps, Camera* pCam, std::string& outScreenShotName)
{
	if (bm.frame < bm.warmUpFrames) // warmup
	{
		++bm.frame;
		return bm.time;
	}
	
	if (bm.time > bm.timeEnd) // are we done yet ?
	{
		fclose(bm.f);

		if (bm.exitWhenTimeEnds)
		{
			PostQuitMessage(0);
			return bm.time;
		}
	}

	SaveTimeStamps(bm.time, timeStamps);

	float time = bm.time;
	bm.time += bm.timeStep;
	return time;
}