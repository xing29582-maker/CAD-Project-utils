#include "RegisterAllCommands.h"

namespace cadutils
{
    void RegisterAllCommands()
    {
        // Intentionally empty.
        // The actual command registration happens via REGISTER_COMMAND macros
        // in each command's .cpp file. The /WHOLEARCHIVE linker flag ensures
        // all static registration variables are linked in from the static library.
        // This function exists as a call-site anchor in main.cpp.
    }
}