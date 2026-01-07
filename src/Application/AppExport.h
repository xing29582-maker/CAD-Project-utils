#pragma once

#if defined(_WIN32)
#if defined(cadutils_application_EXPORTS)
#define CADUTILS_APP_API __declspec(dllexport)
#else
#define CADUTILS_APP_API __declspec(dllimport)
#endif
#else
#define CADUTILS_APP_API
#endif