#pragma once
#include <dxgiformat.h>
#include "Common/Misc/ImgLoader.h"

// Loads a DDS file.
class DDSLoader : public ImgLoader
{
public:
			~DDSLoader();
	bool	Load( const char* pFilename, float cutOff, IMG_INFO* pInfo );

	// After calling Load, calls to CopyPixels return each time a lower mip level
	void	CopyPixels(void* pDest, uint32_t stride, uint32_t width, uint32_t height);

private:
	HANDLE m_handle = INVALID_HANDLE_VALUE;
};