#pragma once

#include "idlgen.h"
#include <cstdint>

namespace winrt::Root
{
	struct SomeInterface : idlgen::author_interface
	{
		virtual void Method() = 0;
		virtual int32_t AnotherMethod(int32_t a) = 0;
		virtual int32_t PropLikeMethod() = 0;
		virtual void PropLikeMethod(int32_t a) = 0;
		void NonPureVirtual();
	private:
		void PrivateMethod();
	};
}
