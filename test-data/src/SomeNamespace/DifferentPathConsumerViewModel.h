#pragma once

#include "idlgen.h"
#include "DifferentPathViewModel.h"
#include "SomeNamespace/DifferentPathConsumerViewModel.g.h"

namespace winrt::Root::SomeNamespace::implementation
{
	struct DifferentPathConsumerViewModel : DifferentPathConsumerViewModelT<DifferentPathConsumerViewModel>, idlgen::author_class
	{
		void Set(Root::SomeNamespace::DifferentPathViewModel a);
	};
}
