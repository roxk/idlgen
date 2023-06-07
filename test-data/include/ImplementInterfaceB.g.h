#pragma once

#include "winrt/Root.h"
#include "Class.h"
#include <cstdint>

namespace winrt::Root
{
	template<typename T, typename... I>
	struct ImplementInterfaceBT_base
	{
	};

	template <typename T, typename... I>
	using ImplementInterfaceBT = ImplementInterfaceBT_base<T, I...>;
}