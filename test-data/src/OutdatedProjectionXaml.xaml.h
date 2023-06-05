#pragma once

#include "OutdatedProjectionXaml.g.h"

namespace winrt::Root::implementation
{
	struct OutdatedProjectionXaml : OutdatedProjectionXamlT<OutdatedProjectionXaml>
	{
		void ProjectedSelf(Root::OutdatedProjectionXaml const& a);
	};

	void InstantiateOutdatedProjection()
	{
		OutdatedProjectionXaml proj;
	}
}
