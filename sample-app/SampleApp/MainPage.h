#pragma once

#include "pch.h"
#include "MainPage.g.h"
#include <algorithm>

namespace winrt::SampleApp::implementation
{
    struct
    [[clang::annotate("idlgen::import=Status.idl")]]
    MainPage : MainPageT<MainPage>, idlgen::base<Windows::UI::Xaml::Controls::Page>
    {
        MainPage() {}

        int32_t MyProperty();
        void MyProperty(int32_t value);

        Status Status() { return mStatus; }
    private:
        friend struct MainPageT<MainPage>;
        uint32_t MinMaxWorks(uint32_t a) { return std::max(a, 42u); }
        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
        SampleApp::Status mStatus{ Status::Unknown };
    };
}

namespace winrt::SampleApp::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
