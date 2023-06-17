#pragma once

#include "winrt/Root.h"
#include <cstdint>

namespace winrt::Root::implementation
{
	namespace impl
	{
		struct OutdatedProjectionXamlAbi
		{
			virtual int32_t RemovedMethod(void* result) = 0;
		};

		template<typename T>
		struct OutdatedProjectionXamlProduce : impl::OutdatedProjectionXamlAbi
		{
			int32_t RemovedMethod(void* result) final
			{
				static_cast<T&>(*this).RemovedMethod();
				return 0;
			}
		};
	}

	template<typename T, typename... I>
	struct OutdatedProjectionXaml_base : impl::OutdatedProjectionXamlProduce<T>
	{
	};
}

#if defined(WINRT_FORCE_INCLUDE_OUTDATEDPROJECTIONXAML_XAML_G_H) || __has_include("OutdatedProjectionXaml.xaml.g.h")

#include "OutdatedProjectionXaml.xaml.g.h"

#else

namespace winrt::Root::implementation
{
	template <typename T, typename... I>
	using OutdatedProjectionXamlT = OutdatedProjectionXaml_base<T, I...>;
}

#endif