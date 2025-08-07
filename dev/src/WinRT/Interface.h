#pragma once

#include "Type.h"
#include <vector>
#include "Method.h"

namespace idlgen::WinRT
{
class Interface : public Type
	{
  private:
        std::vector<std::reference_wrapper<Interface>> _requires;
    std::vector<std::reference_wrapper<Method>> _methods;
      public:
		  EntityKind Kind() const override
		  {
              return EntityKind::Interface;
		}
          void AddRequire(Interface& interface);
          const std::vector<std::reference_wrapper<Interface>> Requries() const
          {
              return _requires;
        }
          void AddMethod(Method& method);
          const std::vector<std::reference_wrapper<Method>> Methods() const
          {
              return _methods;
        }
	};
}