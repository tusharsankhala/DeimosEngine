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
//  Reads a file into a buffer
//
bool ReadFile(const char* name, char** data, size_t* size, bool isbinary)
{
	FILE* file;

	// Open file.
	if (fopen_s(&file, name, isbinary ? "rb" : "r") != 0)
	{
		return false;
	}

	// Get file length.
	fseek(file, 0, SEEK_END);
	size_t fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	// If ascii add one more char to accomodate for the \0
	if (!isbinary)
	{
		fileLen++;
	}

	// Allocate memory
	char* buffer = (char*)malloc(std::max<size_t>(fileLen, 1));
	if (!buffer)
	{
		fclose(file);
		return false;
	}

	// Read file contents into buffer.
	size_t bytesRead = 0;
	if (fileLen > 0)
	{
		bytesRead = fread(buffer, 1, fileLen, file);
	}
	fclose(file);

	if (!isbinary)
	{
		buffer[bytesRead] = 0;
		fileLen = bytesRead;
	}

	*data = buffer;
	if (size != NULL)
		*size = fileLen;

	return true;
}

bool SaveFile(const char* name, void const* data, size_t size, bool isbinary)
{
	FILE* file;

	// Open file.
	if (fopen_s(&file, name, isbinary ? "wb" : "w") == 0)
	{
		fwrite(data, size, 1, file);
		fclose(file);
		return true;
	}

	return true;
}

//
// Launch a process, captures stderr into a file
//
bool LaunchProcess(const char* commandLine, const char* filenameErr)
{
	char cmdLine[1024];
	strcpy_s<1024>(cmdLine, commandLine);

	// create a pipe to get possible errors from the process
	//
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		return false;

	// launch process
	//
	PROCESS_INFORMATION pi = {};
	STARTUPINFOA si = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdError = g_hChildStd_OUT_Wr;
	si.hStdOutput = g_hChildStd_OUT_Wr;
	si.wShowWindow = SW_HIDE;

	if (CreateProcessA(NULL, cmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(g_hChildStd_OUT_Wr);

		ULONG rc;
		if (GetExitCodeProcess(pi.hProcess, &rc))
		{
			if (rc == 0)
			{
				DeleteFileA(filenameErr);
				return true;
			}
			else
			{
				Trace(format("*** Process %s returned an error, see %s ***\n\n", commandLine, filenameErr));

				// save errors to disk
				std::ofstream ofs(filenameErr, std::ofstream::out);

				for (;;)
				{
					DWORD dwRead;
					char chBuf[2049];
					BOOL bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, 2048, &dwRead, NULL);
					chBuf[dwRead] = 0;
					if (!bSuccess || dwRead == 0) break;

					Trace(chBuf);

					ofs << chBuf;
				}

				ofs.close();
			}
		}

		CloseHandle(g_hChildStd_OUT_Rd);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		Trace(format("*** Can't launch: %s \n", commandLine));
	}

	return false;
}

//
// Frustum culls an AABB. The culling is done in clip space
bool CameraFrustumToboxCollision(const math::Matrix4& mCameraViewProj, const math::Vector4& boxCenter, const math::Vector4& boxExtent)
{
	float ex = boxExtent.getX();
	float ey = boxExtent.getY();
	float ez = boxExtent.getZ();

	math::Vector4 p[8];
	p[0] = mCameraViewProj * (boxCenter + math::Vector4(ex, ey, ez, 0));
	p[1] = mCameraViewProj * (boxCenter + math::Vector4(ex, ey, -ez, 0));
	p[2] = mCameraViewProj * (boxCenter + math::Vector4(ex, -ey, ez, 0));
	p[3] = mCameraViewProj * (boxCenter + math::Vector4(ex, -ey, -ez, 0));
	p[4] = mCameraViewProj * (boxCenter + math::Vector4(-ex, ey, ez, 0));
	p[5] = mCameraViewProj * (boxCenter + math::Vector4(-ex, ey, -ez, 0));
	p[6] = mCameraViewProj * (boxCenter + math::Vector4(-ex, -ey, ez, 0));
	p[7] = mCameraViewProj * (boxCenter + math::Vector4(-ex, -ey, -ez, 0));

	uint32_t left = 0;
	uint32_t right = 0;
	uint32_t top = 0;
	uint32_t bottom = 0;
	uint32_t back = 0;

	for (int i = 0; i < 8; ++i)
	{
		float x = p[i].getX();
		float y = p[i].getY();
		float z = p[i].getZ();
		float w = p[i].getW();

		if (x < -w) left++;
		if (x > w) right++;
		if (y < -w) bottom++;
		if (y > w) top++;
		if (z < 0) back++;
	}

	return left == 8 || right == 8 || top == 8 || bottom == 8 || back == 8;
}

AxisAlignedBoundingBox::AxisAlignedBoundingBox()
	: m_min()
	, m_max()
	, m_isEmpty{ true }
{}

void AxisAlignedBoundingBox::Merge(const AxisAlignedBoundingBox& bb)
{
	if (bb.m_isEmpty)
		return;

	if (m_isEmpty)
	{
		m_max = bb.m_max;
		m_min = bb.m_min;
		m_isEmpty = false;
	}
	else
	{
		m_min = Vectormath::SSE::minPerElem(m_min, bb.m_min);
		m_max = Vectormath::SSE::minPerElem(m_max, bb.m_max);
	}
}

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

Log* Log::m_pLogInstance = nullptr;

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

