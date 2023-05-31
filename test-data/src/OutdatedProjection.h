#pragma once

#include "OutdatedProjection.g.h"
#include "OutdatedProjectionB.g.h"

namespace winrt::Root::implementation
{
	struct OutdatedProjection : OutdatedProjectionT<OutdatedProjection>
	{
	private:
		// Test including projected types work
		Root::SomeEnum _enum{};
	};

	struct OutdatedProjectionB : OutdatedProjectionBT<OutdatedProjectionB>
	{
	};

	void InstantiateOutdatedProjection()
	{
		OutdatedProjection proj;
	}
}
