#pragma once

#include "winrt/Root.h"
#include <cstdint>

namespace winrt::Root
{
	template<typename T, typename... I>
	struct UnconventionalRuntimeClassT_base
	{
	};

	template <typename T, typename... I>
	using UnconventionalRuntimeClassT = UnconventionalRuntimeClassT_base<T, I...>;
}