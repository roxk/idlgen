#pragma once

#include <cstdint>
#include "base.h"

namespace winrt::Root
{
	struct ShallowerViewModel : Base {};

	struct IncompleteViewModel;

	namespace A
	{
		struct BlankPage : Base {};

		struct SameViewModel : Base {};

		enum class Category : int32_t
		{
			Doc,
			Sample
		};
	}

	namespace B
	{
		struct SiblingViewModel : Base {};
	}
}