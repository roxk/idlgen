#include "RuntimeClass.h"

void idlgen::WinRT::RuntimeClass::AddMethod(std::unique_ptr<ClassMethod> member)
{
    _members.emplace_back(std::move(member));
}

void idlgen::WinRT::RuntimeClass::AddProperty(std::unique_ptr<Property> member)
{
    _members.emplace_back(std::move(member));
}
