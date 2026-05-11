#include "pch.h"
#include "BlankPage.author.h"

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
    Point BlankPage::GetPoint()
    {
        return Point();
    }
    void BlankPage::AssignHandler(winrt::SampleApp::BlankPageHandler const& handler)
    {}
    int32_t BlankPage::Property(winrt::author::getter)
    {
        throw hresult_not_implemented();
    }

    winrt::author::setter BlankPage::Property(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void BlankPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
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
