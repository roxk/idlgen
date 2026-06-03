#pragma once

#include "winrt/author/base.h"
#include <winrt/impl/Windows.Foundation.0.h>
#include <winrt/impl/Windows.UI.Xaml.Controls.0.h>
#include <winrt/impl/Windows.UI.Xaml.Input.0.h>

namespace winrt::SampleApp
{
	struct BlankPage;
	struct BlankPageHandler;
	enum class Permission : unsigned int;
}

namespace winrt::SampleApp::author
{
	enum class Category : int
	{
		Literature,
		Science
	};

	enum class Permission : unsigned int
	{
		Camera = 0x00000001,
		Microphone = 0x00000002
	};

	struct Point : winrt::author::winrt_struct
	{
		int64_t X;
		int64_t Y;
	};

	struct BlankPage : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page, winrt::Windows::Foundation::IStringable>, winrt::author::unsealed
	{
		enum State
		{
			State1,
			State2
		};
		Category Category(winrt::author::getter = {});
		Permission Permission(winrt::author::getter = {});
		winrt::author::setter Permission(author::Permission permission);
		Point GetPoint();
		void RefConstStruct(const SampleApp::Permission& permission);
		void RefStruct(SampleApp::Permission& permission);
		void PassArrayStruct(array_view<SampleApp::Permission const> permissions);
		void ReceiveArrayStruct(array_view<SampleApp::Permission> permissions);
		void FillArrayStruct(com_array<SampleApp::Permission>& permissions);
		void AssignHandler(winrt::SampleApp::BlankPageHandler const& handler);
		void OnKeyboardAcceleratorInvoked(winrt::Windows::UI::Xaml::Input::KeyboardAcceleratorInvokedEventArgs const& args, winrt::author::override = {});
		int32_t Property(winrt::author::getter = {});
		winrt::author::setter Property(int32_t value);
		void ClickHandler(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& args);
		winrt::hstring ToString(winrt::author::override = {});
	protected:
		bool ProtectedGetter();
	};

	struct BlankPageHandler : winrt::author::delegate
	{
		BlankPageHandler(BlankPage const& sender, uint64_t args);
	};

	struct MoreClass : winrt::author::runtimeclass<>
	{
	protected:
		void Method();
	};

	inline void use_winrt_error_handling()
	{
		winrt::check_bool<bool>(false);
	};
}
