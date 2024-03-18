#pragma once

#include "Common/ShaderCompiler/ShaderCompiler.h"

#include <vector>

bool InitDirectXCompiler();

bool DXCompileToDXO( size_t hash, const char* pSrcCode,
					 const DefineList* pDefines,
					 const char* pEntryPoint,
					 const char* pParams,
					 char** outSpvData,
					 size_t* outSpvSize	);
