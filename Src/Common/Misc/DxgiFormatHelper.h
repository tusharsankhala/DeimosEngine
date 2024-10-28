#pragma once

#include <DXGIFormat.h>

// Return the BPP for a particular format.
size_t BitsPerPixel( DXGI_FORMAT fmt );
size_t GetPixelByteSize( DXGI_FORMAT fmt );

// Convert a *_SRGB format into a non-gamma one.
DXGI_FORMAT ConvertIntoNonGammaFormat(DXGI_FORMAT format);

DXGI_FORMAT ConvertIntoGammaFormat(DXGI_FORMAT format);

DXGI_FORMAT SetFormatGamma(DXGI_FORMAT format, bool addGamma);

bool IsBCFormat( DXGI_FORMAT format );