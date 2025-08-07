#pragma once

#include "Type.h"
#include <optional>

namespace idlgen::WinRT
{
	class Array : public Entity
	{
      private:
        std::optional<std::reference_wrapper<Type>> _type;
      public:
		  EntityKind Kind() const override
		  {
              return EntityKind::Array;
		}
		  void SetType(Type& type)
		  {
              _type = type;
		  }
		  bool HasType() const
          {
              return _type.has_value();
		  }
		  Type& Type() const
		  {
              return _type->get();
		  }
	};
}