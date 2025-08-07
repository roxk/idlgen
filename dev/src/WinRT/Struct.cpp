#include "Struct.h"

void idlgen::WinRT::Struct::AddField(std::string name, Type& type)
{
    _fields.emplace_back(Field{std::move(name), type});
}
