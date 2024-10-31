#pragma once
#include <cstdint>
#include <string>
#include "StringUtil.h"

using sByte = signed char;
using uByte = unsigned char;

using int8 = signed char;
using uint8 = unsigned char;

using int16 = signed short;
using uint16 = unsigned short;

using int32 = signed int;
using uint32 = unsigned int;

#ifdef WINDOWS

using int64 = signed long long;
using uint64 = unsigned long long;

#else

using int64 = signed long;
using uint64 = unsigned long;

#endif
