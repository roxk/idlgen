#include "OutdatedProjection.g.h"

namespace winrt::Root::implementation
{
	struct OutdatedProjection : OutdatedProjectionT<OutdatedProjection>
	{

	};

	void InstantiateOutdatedProjection()
	{
		OutdatedProjection proj;
	}
}
