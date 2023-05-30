#pragma once

#include "winrt/Root.h"
#include "Class.h"

namespace winrt::Root::A
{
	template<typename T, typename... I>
	struct BlankPageT_base
	{
	};

	template <typename T, typename... I>
	using BlankPageT = BlankPageT_base<T, I...>;
}

namespace winrt::Root::A::factory_implementation
{
	template<typename F, typename T>
	struct BlankPageT
	{
	};
}
