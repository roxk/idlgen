#include "EnumFlags.h"

void idlgen::WinRT::EnumFlags::AddEntry(std::string name, int64_t value)
{
    _entries.emplace_back(Entry{std::move(name), value});
}
