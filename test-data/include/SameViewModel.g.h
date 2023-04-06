#pragma once

#include "winrt/Root.h"

namespace winrt::Root::A
{
	template<typename T>
	struct SameViewModelT : winrt::ProduceBase
	{
	};

	template<typename T>
	struct SameViewModelHideT : winrt::ProduceBase
	{
	};
}
