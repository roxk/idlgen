#include "pch.h"
#include "OldSchoolImplementationType.h"
#if __has_include("OldSchoolImplementationType.g.cpp")
#include "OldSchoolImplementationType.g.cpp"
#endif

namespace winrt::SampleApp::implementation
{
    int32_t OldSchoolImplementationType::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void OldSchoolImplementationType::MyProperty(int32_t /*value*/)
    {
        throw hresult_not_implemented();
    }
}
