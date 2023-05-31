#pragma once

#include "winrt/Root.h"
#include "Class.h"
#include <cstdint>

namespace winrt::Root
{
	namespace impl
	{
		struct OutdatedProjectionBAbi
		{
			virtual int32_t RemovedMethod(void* result) = 0;
		};

		template<typename T>
		struct OutdatedProjectionBProduce : impl::OutdatedProjectionBAbi
		{
			int32_t RemovedMethod(void* result) final
			{
				static_cast<T&>(*this).RemovedMethod();
				return 0;
			}
		};
	}

	template<typename T, typename... I>
	struct OutdatedProjectionB_base : impl::OutdatedProjectionBProduce<T>
	{
	};

	template <typename T, typename... I>
	using OutdatedProjectionBT = OutdatedProjectionB_base<T, I...>;
}