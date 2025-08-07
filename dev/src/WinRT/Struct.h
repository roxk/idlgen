#pragma once

#include "Type.h"
#include <vector>

namespace idlgen::WinRT
{
class Struct : public Type
	{
  public:
	  struct Field
	  {
          std::string name;
          Type& type;
	  };

    private:
      std::vector<Field> _fields;
      public:
		  EntityKind Kind() const override
		  {
              return EntityKind::Struct;
		}
          void AddField(std::string name, Type& type);
        const std::vector<Field>& Fields() const
        {
            return _fields;
        }
	};
}