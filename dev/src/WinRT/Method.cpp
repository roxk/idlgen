#include "Method.h"

void idlgen::WinRT::Method::AddParameter(std::string name, Type& type, ParameterKind kind)
{
    _parameters.emplace_back(Parameter{std::move(name), type, kind});
}
