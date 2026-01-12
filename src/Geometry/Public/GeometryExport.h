#pragma once

#if defined(_WIN32)
#if defined(cadutils_geometry_EXPORTS)
#define CADUTILS_GEOMETRY_API __declspec(dllexport)
#else
#define CADUTILS_GEOMETRY_API __declspec(dllimport)
#endif
#else
#define CADUTILS_GEOMETRY_API
#endif