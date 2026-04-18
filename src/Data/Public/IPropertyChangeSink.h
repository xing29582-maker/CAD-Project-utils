#pragma once

#include "NameDefine.h"
#include "DirtyFlags.h"

#include <memory>
#include <string>

namespace cadutils
{
	class IObject;
	class PropertyBase;

	class IPropertyChangeSink
	{
	public:
		virtual ~IPropertyChangeSink() noexcept = default;

		// Property-level notifications
		virtual void OnPropertyChanging(cadutils::ObjectId objId,
			cadutils::PropertyId propId,
			const cadutils::AnyValue& oldValue) = 0;

		virtual void OnPropertyChanged(cadutils::ObjectId objId,
			cadutils::PropertyId propId,
			const cadutils::AnyValue& newValue) = 0;

		// Object-level notifications
		virtual void OnObjectAdded(cadutils::ObjectId objId,
			const std::shared_ptr<IObject>& obj) = 0;

		virtual void OnObjectRemoved(cadutils::ObjectId objId,
			const std::shared_ptr<IObject>& obj) = 0;
	};
}