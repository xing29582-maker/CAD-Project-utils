#pragma once

#include "NameDefine.h"
#include "DirtyFlags.h"

#include <string>
namespace cadutils
{
	class IObject;
	class PropertyBase;

	class IPropertyChangeSink
	{
	public:
		virtual ~IPropertyChangeSink() noexcept = default;
		virtual void OnPropertyChanging(cadutils::ObjectId objId,
			cadutils::PropertyId propId,
			const cadutils::AnyValue& oldValue) = 0;

		virtual void OnPropertyChanged(cadutils::ObjectId objId,
			cadutils::PropertyId propId,
			const cadutils::AnyValue& newValue) = 0;
	};
}