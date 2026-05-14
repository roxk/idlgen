#pragma once

#include "winrt/author/base.h"
#include "winrt/Windows.UI.Xaml.Controls.h"
#include "BlankPage.author.h"
#include "SomeFolder/PageType.h"

namespace winrt::SampleApp::author
{
	struct MainPage : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page>, winrt::author::unsealed
	{
		int32_t MyProperty(winrt::author::getter = {});
		winrt::author::setter MyProperty(int32_t value);
		Category Category(winrt::author::getter = {});
		PageType PageType(winrt::author::getter = {});
		void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args, winrt::author::ignore = {});
	protected:
		int32_t Getter(winrt::author::getter = {});
		void Method();
	};
};