#pragma once

#include "winrt/Root.h"

namespace winrt::Root::SomeNamespace
{
	template<typename T>
	struct DifferentPathConsumerViewModelT : winrt::ProduceBase<>
	{
	};
}
