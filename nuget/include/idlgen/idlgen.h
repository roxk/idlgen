#pragma once

#include <cstdint>

namespace idlgen
{
	template <typename B, typename... I>
	struct base
	{
		// TODO: Verify if we need `no_unique_address`. winrt already applies `empty_bases` on classes
	};

	using enum_normal = int64_t;

	using enum_flag = uint64_t;
}
