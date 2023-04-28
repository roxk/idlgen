#pragma once

#ifdef __clang__
#include <cpuid.h>
#endif

namespace std::chrono
{
	class virtual_base
	{};

	// Simualte non-winrt, non-pod base
	class duration : virtual virtual_base
	{
	public:
		duration() {}
	};

	class time_point : virtual virtual_base
	{
	public:
		time_point() {}
	};
}

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
		using TimeSpan = std::chrono::duration;
		using DateTime = std::chrono::time_point;
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

		template<typename T>
		struct IAsyncOperation : Windows::Foundation::IInspectable
		{
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

		namespace Controls
		{
			struct Page : Base {};
		}

		namespace Data
		{
			struct INotifyPropertyChanged : Base {};
		}
	}
}
