#include "Common/stdafx.h"
#include <d3dcompiler.h>
#include <dxcapi.h>
#include "DXCHelper.h"

#include "Common/Misc/Misc.h"
#include "Common/ShaderCompiler/ShaderCompiler.h"


#define USE_DXC_SPIRV_FROM_DISK

DxcCreateInstanceProc s_dxc_create_func;

bool InitDirectXCompiler()
{
	std::string fullShaderCompilerPath = "dxcompiler.dll";
	std::string fullShaderDXILPath = "dxil.dll";

	HMODULE dxil_module = ::LoadLibrary( fullShaderDXILPath.c_str() );
	HMODULE dxc_module = ::LoadLibrary( fullShaderCompilerPath.c_str() );

	s_dxc_create_func = ( DxcCreateInstanceProc)::GetProcAddress( dxc_module, "DxcCreateInstance" );

	return s_dxc_create_func != NULL;
}