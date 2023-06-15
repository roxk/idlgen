#pragma once

#include "idlgen.h"
#include "SomeNamespace/DifferentPathViewModel.g.h"

namespace winrt::Root::SomeNamespace::implementation
{
	struct DifferentPathViewModel : DifferentPathViewModelT<DifferentPathViewModel>, idlgen::author_class<>
	{
	};
}
