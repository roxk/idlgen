#include "OutdatedProjection.g.h"

namespace winrt::Root::implementation
{
	struct OutdatedProjection : OutdatedProjectionT<OutdatedProjection>
	{
	private:
		Root::SomeEnum _enum{};
	};

	void InstantiateOutdatedProjection()
	{
		OutdatedProjection proj;
	}
}
