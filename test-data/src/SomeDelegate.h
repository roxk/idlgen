#pragma once

#include "ShallowerViewModel.h"
#include "idlgen.h"
#include <cstdint>
#include <winrt/Root.h>

namespace winrt::Root::implementation
{
	struct [[clang::annotate("idlgen::hide")]] HiddenStructHandler : idlgen::author_delegate
	{
		void operator ()(uint64_t s, uint64_t e) {}
	};
	struct HiddenHandler : idlgen::author_delegate
	{
		[[clang::annotate("idlgen::hide")]]
		void operator ()(uint64_t vm, Root::ShallowerViewModel const& e) {}
		void operator ()(Root::ShallowerViewModel const& vm, uint64_t e) {}
	};
	struct NonWinRtTypeHandler : idlgen::author_delegate
	{
		void operator ()(uint64_t vm, Root::implementation::ShallowerViewModel const& e) {}
		void operator ()(Root::ShallowerViewModel const& vm, uint64_t e) {}
	};
	struct
		[[clang::annotate("idlgen::import=SomeFlag.idl")]]
		[[clang::annotate("idlgen::attribute=webhosthidden")]]
		SomeEventHandler : idlgen::author_delegate
	{
		void operator ()(Root::ShallowerViewModel const& vm, uint64_t e) {}
	};
}
