#pragma once
#if WINDOWS && ENGINE_RUN_CODE_ANALYSIS
#include <CodeAnalysis/Warnings.h>

#define CODE_ANALYSIS_BEGIN_EXTERNAL_HEADER _Pragma("warning(push)")\
	 _Pragma("warning(disable: ALL_CODE_ANALYSIS_WARNINGS)")

#define CODE_ANALYSIS_END_EXTERNAL_HEADER _Pragma("warning(pop)")
#else
#define CODE_ANALYSIS_BEGIN_EXTERNAL_HEADER
#define CODE_ANALYSIS_END_EXTERNAL_HEADER
#endif