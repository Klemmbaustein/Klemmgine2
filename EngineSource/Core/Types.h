#pragma once
#include <cstdint>
#include <string>
#include "StringUtil.h"

/**
* @defgroup engine-core Engine core
* 
* @brief
* Core engine functionality, defined in `EngineSource/Core`
* 
* The engine core function as it's own library that functions without the rest of the engine.
* It can be used by linking with the `KlemmgineCore` CMake target.
* 
* The core library contains functions for:
* - Error handling.
* - Serialization.
* - Basic types and structs used by the engine.
* - A thread pool implementation.
* - Basic application features like logging and parsing command line arguments.
*/

/**
* @brief
* Signed byte type.
* 
* @ingroup engine-core
*/
using sByte = signed char;

/**
* @brief
* Unsigned byte type.
*
* @ingroup engine-core
*/
using uByte = unsigned char;

/**
* @brief
* 8 bit integer type. Same as sByte.
*
* @ingroup engine-core
*/
using int8 = signed char;

/**
* @brief
* 8 bit unsigned integer type. Same as uByte.
*
* @ingroup engine-core
*/
using uint8 = unsigned char;

/**
* @brief
* 16 bit integer type.
*
* @ingroup engine-core
*/
using int16 = signed short;

/**
* @brief
* 16 bit unsigned integer type.
*
* @ingroup engine-core
*/
using uint16 = unsigned short;

/**
* @brief
* 32 bit integer type.
*
* @ingroup engine-core
*/
using int32 = signed int;
/**
* @brief
* 32 bit unsigned integer type.
*
* @ingroup engine-core
*/
using uint32 = unsigned int;

#ifdef WINDOWS

/**
* @brief
* 64 bit integer type.
*
* @ingroup engine-core
*/
using int64 = signed long long;
/**
* @brief
* 64 bit unsigned integer type.
*
* @ingroup engine-core
*/
using uint64 = unsigned long long;

#else

/**
* @brief
* 64 bit integer type.
*
* @ingroup engine-core
*/
using int64 = signed long;

/**
* @brief
* 64 bit unsigned integer type.
*
* @ingroup engine-core
*/
using uint64 = unsigned long;

#endif
