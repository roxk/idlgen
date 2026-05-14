#pragma once

#include "winrt/author/base.h"

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
}
