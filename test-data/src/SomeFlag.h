#pragma once

#include "idlgen.h"

using namespace idlgen;

namespace Root
{
	enum class SomeFlag : enum_flag
	{
		Camera = 0x00000001,
		Microphone = 0x0000002
	};
}
