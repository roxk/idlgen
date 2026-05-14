#include "pch.h"
#include "BlankPage.author.h"
#include "winrt/SampleApp.h"
#include "idlgen.impl.h"

using namespace winrt;
using namespace author;
using namespace Windows::UI::Xaml;

namespace winrt::SampleApp::author
{
    //Category BlankPage::Category(winrt::author::getter)
    //{
    //    return Category::Literature;
    //}
    //Permission BlankPage::Permission(winrt::author::getter)
    //{
    //    return Permission::Camera;
    //}
    //Point BlankPage::GetPoint()
    //{
    //    return Point();
    //}
    void BlankPage::AssignHandler(winrt::SampleApp::BlankPageHandler const& handler)
    {
        auto button = self(this)->Button();
        auto existingContent = button.Content();
        auto str = winrt::unbox_value<winrt::hstring>(existingContent);
        button.Content(winrt::box_value(L"hi"));
    }
    int32_t BlankPage::Property(winrt::author::getter)
    {
        return 0;
    }
    winrt::author::setter BlankPage::Property(int32_t /* value */)
    {
        return {};
    }
    void BlankPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        SampleApp::BlankPage blankPage;
        auto i = blankPage.Property();
        blankPage.Property(i);
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
