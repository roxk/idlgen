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

	struct SomeEventHandler : Windows::Foundation::IUnknown {};

	enum class SomeFlag : uint32_t
	{
		Camera = 0x00000001,
		Microphone = 0x0000002
	};

	enum class SomeEnum : int32_t
	{
		Active,
		InActive,
		Unknown
	};

	struct SomeStruct {};

	struct SomeInterface : Windows::Foundation::IInspectable {};
}