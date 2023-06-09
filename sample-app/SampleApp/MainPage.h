﻿#pragma once

#include "pch.h"
#include "BlankPage.h"
#include "MainPage.g.h"
#include "SomeFolder/PageType.h"
#include <algorithm>

namespace winrt::SampleApp::implementation
{
    struct
    [[idlgen::import("Status.idl")]]
    MainPage : MainPageT<MainPage>, idlgen::author_class<Windows::UI::Xaml::Controls::Page>
    {
        MainPage() {}

        [[idlgen::property]]
        int32_t MyProperty();
        [[idlgen::property]]
        void MyProperty(int32_t value);

        Status Status() { return mStatus; }

        PageType PageType();
    protected:
        friend struct winrt::impl::produce<MainPage, SampleApp::IMainPageProtected>;
        int32_t Property();
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
