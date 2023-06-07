#pragma once

#include "winrt/Root.h"
#include "Class.h"
#include <cstdint>

namespace winrt::Root
{
	template<typename T, typename... I>
	struct ImplementInterfaceT_base
	{
	};

	template <typename T, typename... I>
	using ImplementInterfaceT = ImplementInterfaceT_base<T, I...>;
}