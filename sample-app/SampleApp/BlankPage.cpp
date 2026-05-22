#include "pch.h"
#include "BlankPage.author.h"
#include "winrt/SampleApp.h"
#include "idlgen.impl.h"

using namespace winrt;
using namespace author;
using namespace Windows::UI::Xaml;

namespace winrt::SampleApp::author
{
    Category BlankPage::Category(winrt::author::getter)
    {
        return Category::Literature;
    }
    Permission BlankPage::Permission(winrt::author::getter)
    {
        return Permission::Camera;
    }
    winrt::author::setter BlankPage::Permission(author::Permission permission)
    {
        return {};
    }
    Point BlankPage::GetPoint()
    {
        Point pt{};
        pt.X = 42;
        pt.Y = 0;
        return pt;
    }
    void BlankPage::RefConstStruct(const SampleApp::Permission& permission)
    {
    }
    void BlankPage::RefStruct(SampleApp::Permission & permission)
    {}
    void BlankPage::PassArrayStruct(array_view<SampleApp::Permission const> permissions)
    {}
    void BlankPage::ReceiveArrayStruct(array_view<SampleApp::Permission> permissions)
    {}
    void BlankPage::FillArrayStruct(com_array<SampleApp::Permission>& permissions)
    {}
    void BlankPage::AssignHandler(winrt::SampleApp::BlankPageHandler const& handler)
    {
        auto button = self(this)->Button();
        auto existingContent = button.Content();
        auto str = winrt::unbox_value<winrt::hstring>(existingContent);
        button.Content(winrt::box_value(L"hi"));
        self(this)->GetTemplateChild(L"");
        winrt::Windows::Foundation::IStringable stringable = *self(this);
    }
    int32_t BlankPage::Property(winrt::author::getter)
    {
        return 0;
    }
    winrt::author::setter BlankPage::Property(int32_t /* value */)
    {
        return {};
    }
    void BlankPage::ClickHandler(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::RoutedEventArgs const&)
    {
        SampleApp::BlankPage blankPage;
        auto i = blankPage.Property();
        blankPage.Property(i);
        auto pt = blankPage.GetPoint();
        AssignHandler([](auto, auto) {});
    }
    bool BlankPage::ProtectedGetter()
    {
        return false;
    }
    BlankPageHandler::BlankPageHandler(BlankPage const& sender, uint64_t args)
    {}
    void MoreClass::Method()
    {}
}
