#pragma once

#include <DXGIFormat.h>

// Return the BPP for a particular format.
size_t BitsPerPixel( DXGI_FORMAT fmt );
size_t GetPixelByteSize( DXGI_FORMAT fmt );

bool IsBCFormat( DXGI_FORMAT format );