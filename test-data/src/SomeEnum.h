#pragma once

#include "idlgen.h"

using namespace idlgen;

namespace winrt::Root::implementation
{
	enum class SomeEnum : author_enum
	{
		Active,
		InActive,
		Unknown
	};
}
