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
		struct IUnknown
		{
			virtual void Release() = 0;
			virtual void AddRef() = 0;
		};
		struct IInspectable : Windows::Foundation::IUnknown {};
	}
	struct Base : Windows::Foundation::IInspectable
	{
		void Release() {}
		void AddRef() {}
	};
	struct ProduceBase : Windows::Foundation::IInspectable
	{
		void Release() {}
		void AddRef() override {}
	};
	namespace Windows::Foundation
	{
		struct EventHandler : Windows::Foundation::IUnknown {};

		template<typename T, typename A>
		struct TypedEventHandler : Windows::Foundation::IUnknown
		{
			void operator()(const T& sender, const A& args) {}
		};

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
		struct DependencyProperty : Base {};
	}
}
