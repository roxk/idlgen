#pragma once

#include "winrt/Root.h"
#include "Class.h"

namespace winrt::Root::A
{
	template<typename T>
	struct BlankPageT
	{
	};
}

namespace winrt::Root::A::factory_implementation
{
	template<typename F, typename T>
	struct BlankPageT
	{
	};
}
