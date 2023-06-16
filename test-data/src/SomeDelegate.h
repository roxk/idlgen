#pragma once

#include "ShallowerViewModel.h"
#include "idlgen.h"
#include <cstdint>
#include <winrt/Root.h>

namespace winrt::Root::implementation
{
	struct [[idlgen::hide]] _HiddenStructHandler : idlgen::author_delegate<void, uint64_t, uint64_t>
	{
	};
	struct _NonWinRtTypeHandler : idlgen::author_delegate<void, uint64_t, Root::implementation::ShallowerViewModel>
	{
	};
	struct
		[[idlgen::import("SomeFlag.idl")]]
		[[idlgen::attribute("webhosthidden")]]
		_SomeEventHandler : idlgen::author_delegate<void, Root::ShallowerViewModel, uint64_t>
	{
	};
}
