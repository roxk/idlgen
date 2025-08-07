#pragma once

#include "Type.h"

namespace idlgen::WinRT
{
	enum class PrimitiveKind
	{
		Int16,
		Int32,
		Int64,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		Single,
		Double,
		Boolean
	};

	class Primitive : public Type
	{
      private:
        PrimitiveKind _kind;
      public:
        Primitive(PrimitiveKind kind) : _kind(kind)
		{
		}
		EntityKind Kind() const override
		{
            return EntityKind::Primitive;
		}
        PrimitiveKind PrimitiveKind() const
        {
            return _kind;
        }
	};
}