#include "Common/stdafx.h"
#include "Misc.h"

//
// Get current time in milliseconds.
//
double MillisecondsNow()
{
	static LARGE_INTEGER s_frequency;
	static bool s_use_qpc = QueryPerformanceFrequency( &s_frequency );
	double milliseconds = 0;

	if( s_use_qpc )
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter( &now );
		milliseconds = double( 1000.0 * now.QuadPart) / s_frequency.QuadPart;
	}
	else
	{
		milliseconds = double( GetTickCount() );
	}

	return milliseconds;
}

class MessageBuffer
{
public:
	MessageBuffer( size_t len ) :
		m_dynamic( len > STATIC_LEN ? len : 0 ),
		m_ptr( len > STATIC_LEN ? m_dynamic.data() : m_static )
	{
	}

	char* Data() { return m_ptr; }
private:
	static const size_t STATIC_LEN = 256;
	char m_static[ STATIC_LEN ];
	std::vector<char> m_dynamic;
	char* m_ptr;
};

//
// Formats a string.
//

std::string format(const char* format, ...)
{
	va_list args;
	va_start (args, format);
#ifndef _MSC_VER
	size_t size = std::snprintf( nullptr, 0, format, args ) + 1; // Extra space for '\0'
	MessageBuffer buf(size);
	std::vsnprintf( buf.Data(), size, format, args );
	va_end( args );
	return std::string( buf.Data(), buf.Data() + size - 1); // We don't want the '\0' inside.
#else
	const size_t size = (size_t)_vscprintf( format, args) + 1;
	MessageBuffer buf(size);
	vsnprintf_s( buf.Data(), size, _TRUNCATE, format, args);
	va_end( args );
	return std::string( buf.Data(), buf.Data() + size - 1); // We don't want the '\0' inside.
#endif
}

int countBits( uint32_t v)
{
	v = v - ((v >> 1) & 0x55555555);                // put count of each 2 bits into those 2 bits
	v = (v & 0x33333333) + ((v >> 2) & 0x33333333); // put count of each 4 bits into those 4 bits  
	return ((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
}

Log::Log()
{
	PWSTR path = NULL;
	SHGetKnownFolderPath( FOLDERID_LocalAppData, 0, NULL, &path );
	CreateDirectoryW( ( std::wstring(path) + L"\\ENGINE").c_str(), 0 );
	CreateDirectoryW( ( std::wstring(path) + L"\\ENGINE\\DeimosEngine").c_str(), 0);

	m_fileHandle = CreateFileW( (std::wstring(path) + L"\\ENGINE\\DeimosEngine\\DeimosEngine.Log").c_str(), GENERIC_WRITE,
								 FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, nullptr );
	assert( m_fileHandle != INVALID_HANDLE_VALUE );

	// Initialize the overlapped structure for asynchronous write.
	for( uint32_t i = 0; i < MAX_INFLIGHT_WRITES; ++i )
		m_overlappedData[i] = { 0 };

}

Log::~Log()
{
	CloseHandle( m_fileHandle );
	m_fileHandle = INVALID_HANDLE_VALUE;
}

void OverlappedCompletionRoutine( DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
	// We never go into an alert state, so this is just to compile
}

int Log::InitLogSystem()
{
	// Create an instance of the log system if not already exists.
	if( !m_pLogInstance )
	{
		m_pLogInstance = new Log();
		assert( m_pLogInstance );
		if( m_pLogInstance )
			return 0;
	}

	// Something went wrong.
	return -1;
}

int Log::TerminateLogSystem()
{
	if( m_pLogInstance )
	{
		delete m_pLogInstance;
		m_pLogInstance = nullptr;
		return 0;
	}

	// Something went wrong.
	return -1;
}

void Trace(const std::string& str)
{
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);

	// Output to attached debugger.
	OutputDebugStringA(str.c_str());

	// Also log to a file.
	Log::Trace(str.c_str());
}

void Trace(const char* pFormat, ...)
{
	std::mutex mutex;
	std::unique_lock<std::mutex> lock(mutex);

	va_list args;

	// generate formatted string
	va_start(args, pFormat);
	const size_t bufLen = (size_t)_vscprintf(pFormat, args) + 2;
	MessageBuffer buf(bufLen);
	vsnprintf_s(buf.Data(), bufLen, bufLen, pFormat, args);
	va_end(args);
	strcat_s(buf.Data(), bufLen, "\n");

	// Output to attached debugger
	OutputDebugStringA(buf.Data());

	// Also log to file
	Log::Trace(buf.Data());
}

void Log::Trace( const char* logString )
{
	assert( m_pLogInstance );	// Can't do anything without a valid instance.
	if( m_pLogInstance )
	{
		m_pLogInstance->Write( logString );
	}
}

void Log::Write( const char* logString )
{
	OVERLAPPED* pOverlapped = &m_overlappedData[ m_currentIOBufferIndex];

	bool result = WriteFileEx( m_fileHandle, logString, static_cast<DWORD>(strlen(logString)), pOverlapped, OverlappedCompletionRoutine );
	assert( result );
}

