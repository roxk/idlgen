#pragma once

#include "winrt/Root.h"
#include <cstdint>

namespace winrt::Root
{
	template<typename T, typename... I>
	struct ImplementMoreInterfaceT_base
	{
	};

	template <typename T, typename... I>
	using ImplementMoreInterfaceT = ImplementMoreInterfaceT_base<T, I...>;
}