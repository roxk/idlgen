#pragma once

#include "OldSchoolImplementationType.g.h"

namespace winrt::SampleApp::implementation
{
    struct OldSchoolImplementationType : OldSchoolImplementationTypeT<OldSchoolImplementationType>
    {
        OldSchoolImplementationType() = default;

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::SampleApp::factory_implementation
{
    struct OldSchoolImplementationType : OldSchoolImplementationTypeT<OldSchoolImplementationType, implementation::OldSchoolImplementationType>
    {
    };
}
