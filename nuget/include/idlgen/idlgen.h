#pragma once

#include <cstdint>

namespace idlgen
{
	template <typename B, typename... I>
	struct base
	{
		// TODO: Verify if we need `no_unique_address`. winrt already applies `empty_bases` on classes
	};

	using author_enum = int64_t;

	using author_enum_flags = uint64_t;

	struct author_struct
	{
	};

	struct author_delegate
	{
	};
	
	struct author_interface
	{
	};
}
