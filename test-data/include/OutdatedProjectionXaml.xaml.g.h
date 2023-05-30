#pragma once

namespace winrt::Root::implementation
{
	template <typename D, typename... I>
	struct OutdatedProjectionXamlT : OutdatedProjectionXaml_base<D, I...>
	{
		using base_type = typename OutdatedProjectionXamlT::base_type;
		using base_type::base_type;
		using class_type = typename OutdatedProjectionXamlT::class_type;
	};
}
