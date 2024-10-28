#pragma once
#include <string>

bool InitShaderCompilerCache(const std::string shaderLibDir, std::string shaderCacheDir);
std::string GetShaderCompilerLibDir();
std::string GetShaderCompilerCacheDir();