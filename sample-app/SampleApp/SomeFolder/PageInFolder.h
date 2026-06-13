#pragma once

#include <winrt/author/base.h>
#include <winrt/impl/Windows.UI.Xaml.Controls.0.h>
#include "../BlankPage.h"

namespace winrt::SampleApp::author
{
    struct PageInFolder : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page>
    {
        int32_t MyProperty(winrt::author::getter = {});
        winrt::author::setter MyProperty(int32_t value);
    };
}
