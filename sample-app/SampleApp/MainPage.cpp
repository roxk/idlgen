﻿#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::SampleApp::implementation
{
    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    PageType MainPage::PageType()
    {
        return PageType::Main;
    }

    int32_t MainPage::Property()
    {
        return 0;
    }

    void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}
