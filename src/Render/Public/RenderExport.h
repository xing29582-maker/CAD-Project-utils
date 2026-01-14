#pragma once

#if defined(_WIN32)
#if defined(cadutils_render_EXPORTS)
#define CADUTILS_RENDER_API __declspec(dllexport)
#else
#define CADUTILS_RENDER_API __declspec(dllimport)
#endif
#else
#define CADUTILS_RENDER_API
#endif