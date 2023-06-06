#pragma once

#include "SameViewModel.g.h"
#include "SealedSameViewModel.g.h"
#include "SameViewModelHide.g.h"

namespace winrt::Root::A::implementation
{
	class SameViewModel : public SameViewModelT<SameViewModel>
	{
	};
	
	class [[idlgen::sealed]] 
		SealedSameViewModel : public SealedSameViewModelT<SealedSameViewModel>
	{

	};

	class [[idlgen::hide]]
		SameViewModelHide : public SameViewModelHideT<SameViewModelHide>
	{

	};
}
