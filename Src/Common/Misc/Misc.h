#pragma once

#include <vectormath/vectormath.hpp>

static constexpr float PI			= 3.1415926535897932384626433832795f;
static constexpr float PI_OVER_2	= 1.5707963267948966192313216916398f;
static constexpr float PI_OVER_4	= 0.78539816339744830961566084581988f;

double MillisecondsNow();
std::string format( const char* format, ... );
bool ReadFile(const char* name, char** data, size_t* size, bool isbinary);
bool SaveFile(const char* name, void const* data, size_t size, bool isbinary);
void Trace( const std::string& str );
void Trace( const char* pFormat, ... );
bool LaunchProcess(const char* commandLine, const char* filenameErr);



template<typename T> inline T AlignUp( T val, T alignment )
{
	return ( val + alignment - (T)1) & ~(alignment - (T)1);
}

class Log
{
public:
	static int InitLogSystem();
	static int TerminateLogSystem();
	static void Trace( const char* logString );

private:
	static Log* m_pLogInstance;

	Log();
	virtual ~Log();

	void Write(const char* logString);

	HANDLE m_fileHandle = INVALID_HANDLE_VALUE;
	#define MAX_INFLIGHT_WRITES 32

	OVERLAPPED m_overlappedData[ MAX_INFLIGHT_WRITES ];
	uint32_t m_currentIOBufferIndex = 0;

	uint32_t m_writeOffsets = 0;
};

int countBits( uint32_t v );