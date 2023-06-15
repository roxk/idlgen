#pragma once

#include "idlgen.h"
#include "wil/cppwinrt_authoring.h"
#include <winrt/Root.h>
#include <cstdint>

namespace winrt::Root::implementation
{
	struct
		[[idlgen::hide]]
		_HiddenInterface : idlgen::author_interface<>
	{
		void Method();
	};
	struct _BaseInterface : idlgen::author_interface<>
	{
		void BaseMethod();
	};
	struct
		[[idlgen::import("SomeFlag.idl")]]
		[[idlgen::attribute("webhosthidden")]]
		_SomeInterface : idlgen::author_interface<BaseInterface>
	{
		[[idlgen::hide]]
		void HiddenMethod();
		void Method();
		int32_t AnotherMethod(int32_t a);
		int32_t PropLikeMethod();
		void PropLikeMethod(int32_t a);
		[[idlgen::property]]
		int32_t Prop();
		[[idlgen::property]]
		void Prop(int32_t a);
		wil::single_threaded_rw_property<bool> WilProp;
		wil::simple_event<bool> WilEvent;
	private:
		void PrivateMethod();
	};
}
