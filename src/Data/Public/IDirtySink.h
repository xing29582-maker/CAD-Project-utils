#pragma once

#include "NameDefine.h"
#include "DirtyFlags.h"

namespace cadutils
{
	class IDirtySink
	{
	public:
		virtual ~IDirtySink() = default;
		virtual void OnObjectDirty(ObjectId id,  DirtyFlags flags) = 0;
	};
}