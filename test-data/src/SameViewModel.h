#pragma once

#include "idlgen.h"
#include "SameViewModel.g.h"
#include "SealedSameViewModel.g.h"
#include "SameViewModelHide.g.h"

namespace winrt::Root::A::implementation
{
	class SameViewModel : public SameViewModelT<SameViewModel>, idlgen::author_class<>
	{
	};
	
	class [[idlgen::sealed]] 
		SealedSameViewModel : public SealedSameViewModelT<SealedSameViewModel>, idlgen::author_class<>
	{

	};

	class [[idlgen::hide]]
		SameViewModelHide : public SameViewModelHideT<SameViewModelHide>, idlgen::author_class<>
	{

	};
}
