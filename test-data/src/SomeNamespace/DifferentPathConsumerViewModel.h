#include "DifferentPathViewModel.h"
#include "SomeNamespace/DifferentPathConsumerViewModel.g.h"

namespace winrt::Root::SomeNamespace::implementation
{
	struct DifferentPathConsumerViewModel : DifferentPathConsumerViewModelT<DifferentPathConsumerViewModel>
	{
		void Set(Root::SomeNamespace::DifferentPathViewModel a);
	};
}
