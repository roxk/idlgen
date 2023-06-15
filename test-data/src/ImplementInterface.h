#pragma once

#include "ImplementInterface.g.h"
#include "ImplementInterfaceB.g.h"
#include "ImplementInterfaceC.g.h"
#include "ImplementInterfaceD.g.h"
#include "ImplementMoreInterface.g.h"

namespace winrt::Root::implementation
{
	struct ImplementInterface : ImplementInterfaceT<ImplementInterface, SomeInterface>, idlgen::author_class
	{
	};
	struct ImplementInterfaceB : ImplementInterfaceBT<implementation::ImplementInterfaceB, SomeInterface>, idlgen::author_class
	{
	};
	struct ImplementInterfaceC : ImplementInterfaceCT<ImplementInterfaceC, Root::SomeInterface>, idlgen::author_class
	{
	};
	struct ImplementInterfaceD : ImplementInterfaceDT<implementation::ImplementInterfaceD, Root::SomeInterface>, idlgen::author_class
	{
	};
	struct ImplementMoreInterface : ImplementMoreInterfaceT<ImplementMoreInterface, SomeInterface, BaseInterface>, idlgen::author_class
	{
	};
}
