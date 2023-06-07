#pragma once

#include "winrt/Root.h"
#include "Class.h"
#include <cstdint>

namespace winrt::Root
{
	template<typename T, typename... I>
	struct ImplementInterfaceDT_base
	{
	};

	template <typename T, typename... I>
	using ImplementInterfaceDT = ImplementInterfaceDT_base<T, I...>;
}