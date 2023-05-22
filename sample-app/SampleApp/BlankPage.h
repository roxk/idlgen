﻿#pragma once

#include "pch.h"
#include "pch2.h"
#include "BlankPage.g.h"

namespace winrt::SampleApp::implementation
{
    enum class Category : idlgen::author_enum
    {
        Literature,
        Science
    };

    enum class Permission : idlgen::author_enum_flags
    {
        Camera = 0x00000001,
        Microphone = 0x00000002
    };

    struct Point : idlgen::author_struct
    {
        int64_t X;
        int64_t Y;
    };

    struct BlankPageEventHandler : idlgen::author_delegate
    {
        void operator()(SampleApp::BlankPage const& sender, uint64_t e) {}
    };

    struct
    BlankPage : BlankPageT<BlankPage>, idlgen::base<Windows::UI::Xaml::Controls::Page>
    {
        BlankPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        [[idlgen::property]]
        SampleApp::Category Category() { return SampleApp::Category::Literature; }

        [[idlgen::property]]
        SampleApp::Permission Permission() { return SampleApp::Permission::Camera | SampleApp::Permission::Microphone; }

        SampleApp::Point GetPoint() { return SampleApp::Point{}; }

        void AssignHandler(SampleApp::BlankPageEventHandler const& handler) {}

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    protected:
        friend struct winrt::impl::produce<BlankPage, SampleApp::IBlankPageProtected>;
        bool ProtectedGetter();
    };
}

namespace winrt::SampleApp::factory_implementation
{
    struct BlankPage : BlankPageT<BlankPage, implementation::BlankPage>
    {
    };
}
