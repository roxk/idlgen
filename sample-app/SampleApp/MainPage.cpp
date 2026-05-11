#include "pch.h"
#include "MainPage.author.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::SampleApp::author
{
    int32_t MainPage::MyProperty(winrt::author::getter)
    {
        throw hresult_not_implemented();
    }
    winrt::author::setter MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
    Category MainPage::Category(winrt::author::getter)
    {
        return Category::Literature;
    }
    PageType MainPage::PageType(winrt::author::getter)
    {
        return PageType::Main;
    }
    int32_t MainPage::Getter(winrt::author::getter)
    {
        return 0;
    }
    void MainPage::Method()
    {}
    void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&, winrt::author::ignore)
    {
    }
}
