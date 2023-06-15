#pragma once

#include "idlgen.h"
#include "TestIncludeInTemplate.g.h"

namespace winrt::Root::A::implementation
{
	class TestIncludeInTemplate : TestIncludeInTemplateT<TestIncludeInTemplate>, idlgen::author_class
	{

	};
}
