#include "Common/stdafx.h"
#include "ShaderCompilerCache.h"

std::string s_shaderLibDir;
std::string s_shaderCacheDir;

bool InitShaderCompilerCache(const std::string shaderLibDir, std::string shaderCacheDir)
{
	s_shaderLibDir = shaderLibDir;
	s_shaderCacheDir = shaderCacheDir;

	return true;
}

std::string GetShaderCompilerLibDir()
{
	return s_shaderLibDir;
}

std::string GetShaderCompilerCacheDir()
{
	return s_shaderCacheDir;
}