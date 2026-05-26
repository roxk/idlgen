#pragma once

#include "winrt/author/base.h"
#include "winrt/Windows.Foundation.h"
#include "BlankPage.author.h"

namespace winrt::SampleApp
{
	struct IAuthoredInterface;
}

namespace winrt::SampleApp::author
{
	struct IAuthoredInterface : winrt::author::winrt_interface
	{
		virtual void Method() = 0;
	};

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
		virtual void OverridableMethod();
		virtual void MethodWithValueType(Permission permission);
		winrt::hstring ToString(winrt::author::ignore = {});
	};

	struct DerivedViewModel : winrt::author::runtimeclass<ViewModelWithInternalInterface>, IAuthoredInterface
	{
		void OverridableMethod(winrt::author::override = {});
		void MethodWithValueType(Permission permission, winrt::author::override = {});
		void Method() override;
	};

    struct ImplementingInternalAuthoredInterface : winrt::author::runtimeclass<winrt::author::internal<SampleApp::IAuthoredInterface>>
    {
        void Method(winrt::author::ignore = {});
    };
}
