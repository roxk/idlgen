#include "Interface.h"

void idlgen::WinRT::Interface::AddMethod(Method& method)
{
    _methods.emplace_back(method);
}
