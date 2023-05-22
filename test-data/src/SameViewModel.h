#include "SameViewModel.g.h"

namespace winrt::Root::A::implementation
{
	class SameViewModel : SameViewModelT<SameViewModel>
	{

	};

	
	class [[idlgen::sealed]] 
		SealedSameViewModel : SealedSameViewModelT<SealedSameViewModel>
	{

	};

	class [[idlgen::hide]]
		SameViewModelHide : SameViewModelHideT<SameViewModelHide>
	{

	};
}
