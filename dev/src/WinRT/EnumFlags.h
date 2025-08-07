#pragma once

#include "Type.h"
#include <string>
#include <cstdint>
#include <vector>

namespace idlgen::WinRT
{
    class EnumFlags : public Type
	{
      public:
        struct Entry
        {
            std::string name;
            int64_t value;
        };
      private:
        std::vector<Entry> _entries;
      public:
		  idlgen::WinRT::EntityKind Kind() const override
		  {
              return EntityKind::EnumFlags;
		}
          void AddEntry(std::string name, int64_t value);
          const std::vector<Entry>& Entries() const
          {
              return _entries;
          }
	};
}