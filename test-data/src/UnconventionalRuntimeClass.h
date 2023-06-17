#pragma once

#include "idlgen.h"
#include "UnconventionalRuntimeClass.g.h"

namespace winrt::Root::A::implementation
{
	class UnconventionalRuntimeClass : public Windows::UI::Xaml::Markup::ComponentConnectorT<UnconventionalRuntimeClassT<UnconventionalRuntimeClass>>, idlgen::author_class<>
	{
	};
}
