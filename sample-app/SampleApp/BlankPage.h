#pragma once

#include "pch.h"
#include "BlankPage.g.h"

namespace winrt::SampleApp::implementation
{
    enum class Category : idlgen::enum_normal
    {
        Literature,
        Science
    };

    enum class Permission : idlgen::enum_flag
    {
        Camera = 0x00000001,
        Microphone = 0x00000002
    };

    struct Point : idlgen::author_struct
    {
        int64_t X;
        int64_t Y;
    };

    struct
    BlankPage : BlankPageT<BlankPage>, idlgen::base<Windows::UI::Xaml::Controls::Page>
    {
        BlankPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        [[clang::annotate("idlgen::property")]]
        SampleApp::Category Category() { return SampleApp::Category::Literature; }

        [[clang::annotate("idlgen::property")]]
        SampleApp::Permission Permission() { return SampleApp::Permission::Camera | SampleApp::Permission::Microphone; }

        SampleApp::Point GetPoint() { return SampleApp::Point{}; }

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::SampleApp::factory_implementation
{
    struct BlankPage : BlankPageT<BlankPage, implementation::BlankPage>
    {
    };
}
