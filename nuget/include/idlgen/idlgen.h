#pragma once

#include <cstdint>

namespace idlgen
{
	using author_enum = int64_t;

	using author_enum_flags = uint64_t;

	struct author_struct
	{
	};

	template<typename R, typename Sender, typename E>
	struct author_delegate
	{
	};

	template <typename... B>
	struct author_interface
	{
		// TODO: Verify if we need `no_unique_address`. winrt already applies `empty_bases` on classes
	};

	template <typename... B>
	struct author_class
	{
		// TODO: Verify if we need `no_unique_address`. winrt already applies `empty_bases` on classes
	};
}
