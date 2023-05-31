#pragma once

#include "SomeNamespace/DifferentPathViewModel.g.h"

namespace winrt::Root::SomeNamespace::implementation
{
	struct DifferentPathViewModel : DifferentPathViewModelT<DifferentPathViewModel>
	{
	};
}
