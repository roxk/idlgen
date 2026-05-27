#pragma once
#include <windows.h>
#include <unknwn.h>
#undef GetCurrentTime
#include <hstring.h>
#include <cstring>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>

#ifndef IDLGEN_CPP_STATIC_REFLECTION_PHASE
#include "OldSchoolImplementationType.h"
#endif

//#include <wil/wistd_type_traits.h>
//#include <wil/cppwinrt_authoring.h>
//
//namespace wil
//{
//	template<typename T>
//	using prop = single_threaded_property<T>;
//
//	template <typename T>
//	using rw_prop = single_threaded_rw_property<T>;
//}
