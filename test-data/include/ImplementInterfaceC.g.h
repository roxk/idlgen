#pragma once

#include "winrt/Root.h"
#include <cstdint>

namespace winrt::Root
{
	template<typename T, typename... I>
	struct ImplementInterfaceCT_base
	{
	};

	template <typename T, typename... I>
	using ImplementInterfaceCT = ImplementInterfaceCT_base<T, I...>;
}