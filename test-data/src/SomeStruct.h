#pragma once

#include "idlgen.h"
#include <cstdint>

namespace winrt::Root
{
	struct SomeStruct : idlgen::author_struct
	{
		int64_t X;
		int64_t Y;
	};
}
