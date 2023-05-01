#include "PropertyBag.g.h"
#include "SameViewModel.h"
#include "wil/cppwinrt_authoring.h"
#include "cppxaml/cppxaml.h"
#include <cstdint>

namespace winrt::Root::A::implementation
{
	struct [[clang::annotate("idlgen::property")]] PropertyBag : PropertyBagT<PropertyBag>
	{
		uint32_t UInt32Prop();
		void UInt32Prop(uint32_t a);
		Root::A::SameViewModel ClassProp();
		[[clang::annotate("idlgen::method")]]
		Windows::Foundation::IAsyncAction MethodAsync();
		void MethodPure();
		void MethodBool(bool a);
		void Method(uint32_t a, uint32_t b);
		bool IsErrored();
		bool IsLoading();
		bool IsIdle();
		cppxaml::XamlProperty<bool> CppXamlProperty;
		wil::single_threaded_property<Root::A::SameViewModel> WilProp;
		wil::single_threaded_rw_property<Root::A::SameViewModel> WilRwProp;
	};
}
