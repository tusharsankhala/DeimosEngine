#include "Common/stdafx.h"
#include "Common/Misc/ImgLoader.h"
#include "Common/Misc/DDSLoader.h"
#include "Common/Misc/WICLoader.h"

ImgLoader* CreateImageLoader( const char* pFilename )
{
	// Get the last 4 char ( assuming this is the file extension )
	size_t len = strlen( pFilename );
	const char* ext = pFilename + ( len - 4 );
	if(_strcmpi( ext, ".dds" ) == 0 )
	{
		return new DDSLoader();
	}
	else
	{
		return new WICLoader();
	}
}

