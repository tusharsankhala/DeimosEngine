#include "Hash.h"

size_t Hash(const void* ptr, size_t size, size_t result)
{
	for (size_t i = 0; i < size; ++i)
	{
		result = (result * 16777619) ^ ((char*)ptr)[i];
	}

	return result;
}

size_t HashString(const char* str, size_t result)
{
	return Hash(str, strlen(str), result);
}

size_t HashString(const std::string& str, size_t result)
{
	return Hash(str.c_str(), result);
}