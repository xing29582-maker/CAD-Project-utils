#pragma once

#if defined(_WIN32)
#if defined(cadutils_platform_EXPORTS)
#define CADUTILS_PLATFORM_API __declspec(dllexport)
#else
#define CADUTILS_PLATFORM_API __declspec(dllimport)
#endif
#else
#define CADUTILS_PLATFORM_API
#endif