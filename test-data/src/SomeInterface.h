#pragma once

#include "idlgen.h"
#include "wil/cppwinrt_authoring.h"
#include <cstdint>

namespace winrt::Root
{
	struct SomeInterface : idlgen::author_interface
	{
		void Method();
		int32_t AnotherMethod(int32_t a);
		int32_t PropLikeMethod();
		void PropLikeMethod(int32_t a);
		wil::single_threaded_rw_property<bool> WilProp;
		wil::simple_event<bool> WilEvent;
	private:
		void PrivateMethod();
	};
}
