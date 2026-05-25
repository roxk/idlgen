#pragma once

#include "winrt/author/base.h"
#include "winrt/Windows.Foundation.h"

namespace winrt::SampleApp::author
{
	struct ViewModel : winrt::author::runtimeclass<>, winrt::author::static_class
	{
		static void StaticMethod1();
		static void StaticMethod2();
	private:
		static void PrivateStaticMethod();
		int PrivateInstanceMethod();
	};

	struct ViewModelWithInternalInterface : winrt::author::runtimeclass<winrt::author::internal<winrt::Windows::Foundation::IStringable>>, winrt::author::unsealed
	{
		void SomeMethod();
		virtual void OveridableMethod();
		winrt::hstring ToString(winrt::author::ignore = {});
	};

	struct DerivedViewModel : winrt::author::runtimeclass<ViewModelWithInternalInterface>
	{
		void OverridableMethod(winrt::author::override = {});
	};
}
