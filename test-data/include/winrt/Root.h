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

		struct SameViewModelHide : Base {};

		struct TestIncludeImpl : Base {};

		struct TestIncludeInTemplate : Base {};

		enum class Category : int32_t
		{
			Doc,
			Sample
		};

		struct PropertyBag : Base {};
	}

	namespace B
	{
		struct SiblingViewModel : Base {};
	}

	namespace SomeNamespace
	{
		struct DifferentPathViewModel : Base {};
		struct DifferentPathConsumerViewModel : Base {};
	}
}