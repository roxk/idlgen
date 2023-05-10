#pragma once

#include "idlgen.h"
#include "wil/cppwinrt_authoring.h"
#include <cstdint>

namespace winrt::Root::implementation
{
	struct
		[[clang::annotate("idlgen::hide")]]
		HiddenInterface : idlgen::author_interface
	{
		void Method();
	};
	struct BaseInterface : idlgen::author_interface
	{
		void BaseMethod();
	};
	struct
		[[clang::annotate("idlgen::import=SomeFlag.idl")]]
		[[clang::annotate("idlgen::attribute=webhosthidden")]]
		SomeInterface : idlgen::author_interface, idlgen::base<Root::BaseInterface>
	{
		[[clang::annotate("idlgen::hide")]]
		void HiddenMethod();
		void Method();
		int32_t AnotherMethod(int32_t a);
		int32_t PropLikeMethod();
		void PropLikeMethod(int32_t a);
		[[clang::annotate("idlgen::property")]]
		int32_t Prop();
		[[clang::annotate("idlgen::property")]]
		void Prop(int32_t a);
		wil::single_threaded_rw_property<bool> WilProp;
		wil::simple_event<bool> WilEvent;
	private:
		void PrivateMethod();
	};
}
