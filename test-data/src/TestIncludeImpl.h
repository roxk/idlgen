#pragma once

#include "idlgen.h"
#include "TestIncludeImpl.g.h"

namespace winrt::Root::A::implementation
{
	class TestIncludeImpl : TestIncludeImplT<TestIncludeImpl>, idlgen::author_class
	{

	};
}
