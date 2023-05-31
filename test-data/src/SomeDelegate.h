#pragma once

#include "ShallowerViewModel.h"
#include "idlgen.h"
#include <cstdint>
#include <winrt/Root.h>

namespace winrt::Root::implementation
{
	struct [[idlgen::hide]] _HiddenStructHandler : idlgen::author_delegate
	{
		void operator ()(uint64_t s, uint64_t e) {}
	};
	struct _HiddenHandler : idlgen::author_delegate
	{
		[[idlgen::hide]]
		void operator ()(uint64_t vm, Root::ShallowerViewModel const& e) {}
		void operator ()(Root::ShallowerViewModel const& vm, uint64_t e) {}
	};
	struct _NonWinRtTypeHandler : idlgen::author_delegate
	{
		void operator ()(uint64_t vm, Root::implementation::ShallowerViewModel const& e) {}
		void operator ()(Root::ShallowerViewModel const& vm, uint64_t e) {}
	};
	struct
		[[idlgen::import("SomeFlag.idl")]]
		[[idlgen::attribute("webhosthidden")]]
		_SomeEventHandler : idlgen::author_delegate
	{
		void operator ()(Root::ShallowerViewModel const& vm, uint64_t e) {}
	};
}
