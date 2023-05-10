#include "SameViewModel.g.h"

namespace winrt::Root::A::implementation
{
	class SameViewModel : SameViewModelT<SameViewModel>
	{

	};

	
	class [[clang::annotate("idlgen::sealed")]] 
		SealedSameViewModel : SealedSameViewModelT<SealedSameViewModel>
	{

	};

	class [[clang::annotate("idlgen::hide")]]
		SameViewModelHide : SameViewModelHideT<SameViewModelHide>
	{

	};
}
