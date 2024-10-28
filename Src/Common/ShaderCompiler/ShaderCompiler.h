#pragma once

#include "Common/Misc/Misc.h"
#include "Common/Misc/Hash.h"
#include <map>

//
// Hash a string of source code and recurse over its #include files.
//
size_t HashShaderString(const char* pRootDir, const char* pShader, size_t result = 2166136261);

//
// DefineList, holds pairs of key & value that will be used by the compiler as defines.
//
class DefineList : public std::map<const std::string, std::string>
{
public:
	bool Has(const std::string& str) const
	{
		return find(str) != end();
	}

	size_t Hash( size_t result = HASH_SEED ) const
	{
		for (auto it = begin(); it != end(); ++it)
		{
			result = ::HashString(it->first, result);
			result = ::HashString(it->second, result);
		}

		return result;
	}

	friend DefineList operator+(DefineList def1,			// passing lhs by value helps optimize chined a+b+c
								const DefineList& def2)		// otherwise, both parameters may be const references
	{
		for (auto it = def2.begin(); it != def2.end(); ++it)
			def1[it->first] = it->second;
		
		return def1;
	}
};