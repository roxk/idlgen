#pragma once

#include "winrt/author/base.h"
#include "BlankPage.author.h"
#include "SomeFolder/PageType.author.h"

namespace winrt::Windows::UI::Xaml::Controls
{
    struct Page;
}

namespace winrt::SampleApp::author
{
	struct MainPage : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page>
	{
		int32_t MyProperty(winrt::author::getter = {});
		winrt::author::setter MyProperty(int32_t value);
		//Category Category(winrt::author::getter = {});
		//PageType PageType(winrt::author::getter = {});
		void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args, winrt::author::ignore = {});
		static int SomeStaticMethod();
	protected:
		int32_t Getter(winrt::author::getter = {});
		void Method();
	};
};