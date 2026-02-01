#pragma once

#include "NameDefine.h"
#include "DirtyFlags.h"

namespace cadutils
{
	class IChangeSink
	{
	public:
		virtual ~IChangeSink() = default;
		virtual void OnPropertyChanged(ObjectId id,  DirtyFlags flags) = 0;
	};
}