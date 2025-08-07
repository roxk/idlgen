#include "Enum.h"

void idlgen::WinRT::Enum::AddEntry(std::string name, uint64_t value)
{
    _entries.emplace_back(Entry{std::move(name), value});
}
