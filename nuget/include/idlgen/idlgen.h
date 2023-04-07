#pragma once

namespace idlgen
{
	template <typename B, typename... I>
	struct base
	{
		// TODO: Verify if we need `no_unique_address`. winrt already applies `empty_bases` on classes
	};
}
