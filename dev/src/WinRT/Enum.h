#pragma once

#include "Type.h"
#include <string>
#include <cstdint>
#include <vector>

namespace idlgen::WinRT
{
	class Enum : public Type
	{
      public:
        struct Entry
        {
            std::string name;
            uint64_t value;
        };
      private:
          std::vector<Entry> _entries; 
      public:
          EntityKind Kind() const override
          {
              return EntityKind::Enum;
        }
        void AddEntry(std::string name, uint64_t value);
        const std::vector<Entry>& Entries() const
        {
            return _entries;
        }
	};
}