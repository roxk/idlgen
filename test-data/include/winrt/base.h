#pragma once

#ifdef __clang__
#include <cpuid.h>
#endif

struct IUnknown
{
	virtual void Release() = 0;
	virtual void AddRef() = 0;
};

namespace winrt
{
	struct hstring {};
	struct event_token {};
	namespace Windows::Foundation
	{
		struct IInspectable : IUnknown
		{
		};
		struct IEventHandler : Windows::Foundation::IInspectable {};
		struct EventHandler : IEventHandler {};

		namespace Numerics
		{
			struct Vector2
			{
				float x;
				float y;
			};
		}
	}

	namespace Windows::UI::Xaml
	{
		struct DependencyProperty : Windows::Foundation::IInspectable {};
	}
}
