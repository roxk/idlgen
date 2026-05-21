#include "pch.h"
#include "MainPage.author.h"
#include "winrt/SampleApp.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::SampleApp::author
{
    int32_t MainPage::MyProperty(winrt::author::getter)
    {
        return 0;
    }
    winrt::author::setter MainPage::MyProperty(int32_t /* value */)
    {
        return {};
    }
    //Category MainPage::Category(winrt::author::getter)
    //{
    //    return Category::Literature;
    //}
    //PageType MainPage::PageType(winrt::author::getter)
    //{
    //    return PageType::Main;
    //}
    int32_t MainPage::Getter(winrt::author::getter)
    {
        return 0;
    }
    void MainPage::Method()
    {}
    void MainPage::ClickHandler(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::RoutedEventArgs const&, winrt::author::ignore)
    {
        SampleApp::MainPage::SomeStaticMethod();
        SampleApp::MainPage mainPage;
        auto i = mainPage.MyProperty();
        mainPage.MyProperty(i);
    }
    int MainPage::SomeStaticMethod()
    {
        return 0;
    }
}
