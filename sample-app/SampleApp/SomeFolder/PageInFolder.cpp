#include "pch.h"
#include "PageInFolder.h"

namespace winrt::SampleApp::author
{
    int32_t PageInFolder::MyProperty(winrt::author::getter)
    {
        throw hresult_not_implemented();
    }

    winrt::author::setter PageInFolder::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
