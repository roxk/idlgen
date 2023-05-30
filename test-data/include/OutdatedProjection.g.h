#pragma once

#include "winrt/Root.h"
#include "Class.h"
#include <cstdint>

namespace winrt::Root
{
	namespace impl
	{
		struct OutdatedProjectionAbi
		{
			virtual int32_t RemovedMethod(void* result) = 0;
		};

		template<typename T>
		struct OutdatedProjectionProduce : impl::OutdatedProjectionAbi
		{
			int32_t RemovedMethod(void* result) final
			{
				static_cast<T&>(*this).RemovedMethod();
				return 0;
			}
		};
	}

	template<typename T, typename... I>
	struct OutdatedProjection_base : impl::OutdatedProjectionProduce<T>
	{
	};

	template <typename T, typename... I>
	using OutdatedProjectionT = OutdatedProjection_base<T, I...>;
}