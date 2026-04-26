#pragma once

#include "DataExport.h"

namespace cadutils
{
    // Register all object types in MetaRegistry
    // Must be called before deserialization
    CADUTILS_DATA_API void RegisterAllTypes();
}