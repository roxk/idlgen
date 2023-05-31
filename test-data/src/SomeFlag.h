#pragma once

#include "idlgen.h"

using namespace idlgen;

namespace winrt::Root::implementation
{
	enum class _SomeFlag : author_enum_flags
	{
		Camera = 0x00000001,
		Microphone = 0x0000002
	};
}
