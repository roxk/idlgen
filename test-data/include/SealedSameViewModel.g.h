#pragma once

#include "winrt/Root.h"

namespace winrt::Root::A
{
	template<typename T>
	struct SealedSameViewModelT : winrt::ProduceBase
	{
	};
}
