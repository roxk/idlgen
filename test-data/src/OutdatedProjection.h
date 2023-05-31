#include "OutdatedProjection.g.h"

namespace winrt::Root::implementation
{
	struct OutdatedProjection : OutdatedProjectionT<OutdatedProjection>
	{
	private:
		// Test including projected types work
		Root::SomeEnum _enum{};
	};

	void InstantiateOutdatedProjection()
	{
		OutdatedProjection proj;
	}
}
