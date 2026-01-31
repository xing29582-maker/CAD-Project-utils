#pragma once

#if defined(_WIN32)
#if defined(cadutils_data_EXPORTS)
#define CADUTILS_DATA_API __declspec(dllexport)
#else
#define CADUTILS_DATA_API __declspec(dllimport)
#endif
#else
#define CADUTILS_DATA_API
#endif