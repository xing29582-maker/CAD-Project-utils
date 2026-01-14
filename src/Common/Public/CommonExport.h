#pragma once

#if defined(_WIN32)
#if defined(cadutils_common_EXPORTS)
#define CADUTILS_COMMON_API __declspec(dllexport)
#else
#define CADUTILS_COMMON_API __declspec(dllimport)
#endif
#else
#define CADUTILS_COMMON_API
#endif