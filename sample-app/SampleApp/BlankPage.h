#pragma once

#include "pch.h"
#include "BlankPage.g.h"
#include "MoreClass.g.h"

namespace winrt::SampleApp::implementation
{
    enum class _Category : idlgen::author_enum
    {
        Literature,
        Science
    };

    enum class _Permission : idlgen::author_enum_flags
    {
        Camera = 0x00000001,
        Microphone = 0x00000002
    };

    struct _Point : idlgen::author_struct
    {
        int64_t X;
        int64_t Y;
    };

    struct _BlankPageEventHandler : idlgen::author_delegate<void, SampleApp::BlankPage, uint64_t>
    {
    };

    struct MoreClass : MoreClassT<MoreClass>, idlgen::author_class<>
    {
        MoreClass() {}
    };

    struct
    BlankPage : BlankPageT<BlankPage>, idlgen::author_class<Windows::UI::Xaml::Controls::Page>
    {
        BlankPage()
        {
            // Xaml objects should not call InitializeComponent during construction.
            // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
        }

        [[idlgen::property]]
        Category Category();

        [[idlgen::property]]
        Permission Permission();

        Point GetPoint() { return Point{}; }

        void AssignHandler(BlankPageEventHandler const& handler) {}

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);

        [[idlgen::protected]]
        bool ProtectedGetter();
    private:
        friend struct winrt::impl::produce<BlankPage, SampleApp::IBlankPageProtected>;
    };
}

namespace winrt::SampleApp::factory_implementation
{
    struct MoreClass : MoreClassT<MoreClass, implementation::MoreClass>
    {
    };
    struct BlankPage : BlankPageT<BlankPage, implementation::BlankPage>
    {
    };
}
