#pragma once

#include "ImplementInterface.g.h"
#include "ImplementMoreInterface.g.h"

namespace winrt::Root::implementation
{
	struct ImplementInterface : ImplementInterfaceT<ImplementInterface, SomeInterface>
	{
	};
	struct ImplementMoreInterface : ImplementMoreInterfaceT<ImplementMoreInterface, SomeInterface, BaseInterface>
	{
	};
}
