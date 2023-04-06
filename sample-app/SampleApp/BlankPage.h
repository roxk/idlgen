#pragma once

#include "pch.h"
#include "BlankPage.g.h"

namespace winrt::SampleApp::implementation
{
    
    struct
    [[clang::annotate("idlgen::extend=Windows.UI.Xaml.Controls.Page")]] 
    BlankPage : BlankPageT<BlankPage>
    {
        BlankPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

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
