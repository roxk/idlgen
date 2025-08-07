#pragma once

#include "Type.h"
#include "Member.h"
#include "ClassMethod.h"
#include "Property.h"
#include <vector>
#include <memory>

namespace idlgen::WinRT
{
	class RuntimeClass : public Type
	{
      private:
        std::vector<std::unique_ptr<Member>> _members;
      public:
		  EntityKind Kind() const override
		  {
              return EntityKind::RuntimeClass;
		}
          void AddMethod(std::unique_ptr<ClassMethod> member);
          void AddProperty(std::unique_ptr<Property> member);
	};
}