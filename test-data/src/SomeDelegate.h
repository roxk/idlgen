#pragma once

#include "idlgen.h"
#include <cstdint>
#include <winrt/Root.h>

namespace winrt::Root
{
	struct SomeEventHandler : idlgen::author_delegate
	{
		void operator ()(Root::ShallowerViewModel const& vm, uint64_t e) {}
	};
}
