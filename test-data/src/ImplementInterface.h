#pragma once

#include "ImplementInterface.g.h"
#include "ImplementInterfaceB.g.h"
#include "ImplementInterfaceC.g.h"
#include "ImplementInterfaceD.g.h"
#include "ImplementMoreInterface.g.h"

namespace winrt::Root::implementation
{
	struct ImplementInterface : ImplementInterfaceT<ImplementInterface, SomeInterface>
	{
	};
	struct ImplementInterfaceB : ImplementInterfaceBT<implementation::ImplementInterfaceB, SomeInterface>
	{
	};
	struct ImplementInterfaceC : ImplementInterfaceCT<ImplementInterfaceC, Root::SomeInterface>
	{
	};
	struct ImplementInterfaceD : ImplementInterfaceDT<implementation::ImplementInterfaceD, Root::SomeInterface>
	{
	};
	struct ImplementMoreInterface : ImplementMoreInterfaceT<ImplementMoreInterface, SomeInterface, BaseInterface>
	{
	};
}
