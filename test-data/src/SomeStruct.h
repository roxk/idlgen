#pragma once

#include "idlgen.h"
#include <cstdint>

namespace winrt::Root::implementation
{
	struct
		[[idlgen::hide]]
		HiddenStruct : idlgen::author_struct
	{
		int64_t X;
		int64_t Y;
	};
	struct
		[[idlgen::import("SomeFlag.idl")]]
		[[idlgen::attribute("webhosthidden")]]
		SomeStruct : idlgen::author_struct
	{
		[[idlgen::hide]]
		int64_t HiddenProp;
		int64_t X;
		int64_t Y;
		Root::ShallowerViewModel DisallowedReferenceTypeGeneratedAsIs;
	private:
		int64_t PrivateProp;
	};
}
