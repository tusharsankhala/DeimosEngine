#include "Common/stdafx.h"
#include "Common/Misc/DDSLoader.h"

DDSLoader::~DDSLoader()
{
	if( m_handle != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_handle );
	}
}

bool DDSLoader::Load(const char* pFilename, float cutOff, IMG_INFO* pInfo)
{

	return true;
}


void DDSLoader::CopyPixels(void* pDest, uint32_t stride, uint32_t bytesWidth, uint32_t height)
{
	assert( m_handle != INVALID_HANDLE_VALUE );
	for( uint32_t y = 0; y < height; ++y )
	{
		ReadFile( m_handle, (char*)pDest + y * stride, bytesWidth, NULL, NULL );
	}
}