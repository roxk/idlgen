#pragma once

#include "Type.h"
#include "Member.h"
#include <optional>

namespace idlgen::WinRT
{
	enum class PropertyKind
	{
		Property,
		Getter
	};

	class Property : public Member
	{
      private:
        PropertyKind _propertyKind;
        std::optional<std::reference_wrapper<Type>> _type;
      public:
		  idlgen::WinRT::MemberKind MemberKind() const override
		  {
              return MemberKind::Property;
		}
		  void SetPropertyKind(PropertyKind kind)
		  {
              _propertyKind = kind;
		  }
		  const idlgen::WinRT::PropertyKind PropertyKind() const
		  {
              return _propertyKind; 
		  }
		  void SetType(Type& type)
		  {
              _type = type;
		  }
		  bool HasType() const
		  {
              return _type.has_value();
		  }
		  Type& Type()
		  {
              return _type->get();
		  }
	};
}