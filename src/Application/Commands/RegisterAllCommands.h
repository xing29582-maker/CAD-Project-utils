#pragma once

// This header forces the linker to include all command registration
// static variables. Include this in main.cpp or any translation unit
// that is guaranteed to be linked.
//
// Each command .cpp file contains a REGISTER_COMMAND macro that creates
// a static CommandRegistrar. In a STATIC library, the linker may discard
// these if nothing else in the translation unit is referenced.
// Including the command headers here and referencing a dummy symbol
// ensures they are linked.

namespace cadutils
{
    // Call this function once at startup to ensure all command
    // registrations are linked in. Implementation is in RegisterAllCommands.cpp.
    void RegisterAllCommands();
}