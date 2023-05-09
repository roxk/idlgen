#pragma once

#include "idlgen.h"
#include <cstdint>

namespace winrt::Root::implementation
{
	struct
		[[clang::annotate("idlgen::hide")]]
		HiddenStruct : idlgen::author_struct
	{
		int64_t X;
		int64_t Y;
	};
	struct
		[[clang::annotate("idlgen::import=SomeFlag.idl")]]
		[[clang::annotate("idlgen::attribute=webhosthidden")]]
		SomeStruct : idlgen::author_struct
	{
		[[clang::annotate("idlgen::hide")]]
		int64_t HiddenProp;
		int64_t X;
		int64_t Y;
		Root::ShallowerViewModel DisallowedReferenceTypeGeneratedAsIs;
	private:
		int64_t PrivateProp;
	};
}
