#include "RegisterAllTypes.h"
#include "SphereObject.h"
#include "BoxObject.h"
#include "CylinderObject.h"

namespace cadutils
{
    void RegisterAllTypes()
    {
        // Force static initialization of all object types
        // This ensures they are registered in MetaRegistry
        (void)SphereObject::StaticTypeMeta();
        (void)BoxObject::StaticTypeMeta();
        (void)CylinderObject::StaticTypeMeta();
    }
}