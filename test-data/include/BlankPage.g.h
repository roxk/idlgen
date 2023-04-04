#pragma once

#include "winrt/Root.h"
#include "Class.h"

namespace winrt::Root::A
{
	template<typename T>
	struct BlankPageT_base
	{
	};

	template <typename T>
	using BlankPageT = BlankPageT_base<T>;
}

namespace winrt::Root::A::factory_implementation
{
	template<typename F, typename T>
	struct BlankPageT
	{
	};
}
