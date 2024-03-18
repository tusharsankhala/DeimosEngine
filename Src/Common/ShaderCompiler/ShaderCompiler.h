#pragma once

#include "Common/Misc/Misc.h"
#include "Common/Misc/Hash.h"
#include <map>

//
// DefineList, holds pairs of key & value that will be used by the compiler as defines.
//
class DefineList : public std::map<const std::string, std::string>
{

};