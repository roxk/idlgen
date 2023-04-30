#include "PropertyBag.g.h"
#include "SameViewModel.h"
#include <cstdint>

namespace winrt::Root::A::implementation
{
	struct [[clang::annotate("idlgen::property")]] PropertyBag : PropertyBagT<PropertyBag>
	{
		uint32_t UInt32Prop();
		void UInt32Prop(uint32_t a);
		Root::A::SameViewModel ClassProp();
		void Method(uint32_t a, uint32_t b);
		bool IsErrored();
		bool IsLoading();
		bool IsIdle();
	};
}
