#pragma once

#include "pch.h"
#include "SomeEnum.h"
#include "SomeDelegate.h"
#include "SomeStruct.h"
#include "SomeInterface.h"
#include "BlankPage.g.h"
// Test inconsistent file name case
#include "sameviewmodel.h"
#include "TestIncludeImpl.h"
#include "TestIncludeInTemplate.h"
#include "SomeNamespace/DifferentPathViewModel.h"
#include "wil/cppwinrt_authoring.h"
#include "cppxaml/cppxaml.h"
#include <cstdint>

namespace winrt::Root::A::implementation
{
	enum TestEnumInMethod : int32_t
	{
		A,
		B
	};

	// Make sure we don't crash when checking against template parameter (not all args are type)
	template <typename... T>
	struct PackedTemplate {};

	template <int I>
	struct IntegerTemplate {};

	struct ImplStruct : PackedTemplate<ImplStruct>, IntegerTemplate<10> {};

	struct Interface
	{
		virtual void MethodOverriden(uint32_t a) = 0;
	};

	struct
		[[idlgen::import("SameViewModel.idl", "ShallowerViewModel.idl",
			"SiblingViewModel.idl")]]
	[[idlgen::attribute("bindable")]]
	[[idlgen::attribute("default_interface")]]
	[[idlgen::attribute("Windows.UI.Xaml.Markup.ContentProperty(\"Property\")")]]
	BlankPage : BlankPageT<BlankPage>, idlgen::base<Windows::UI::Xaml::Controls::Page, Windows::UI::Xaml::Data::INotifyPropertyChanged>, Interface
	{
		BlankPage();
		BlankPage(uint64_t a);
		BlankPage(uint64_t a, uint64_t b);
		BlankPage(ImplStruct s);
		BlankPage(BlankPage const& that) = default;
		BlankPage(BlankPage&& that) = default;
		BlankPage& operator=(BlankPage const& that) = default;
		BlankPage& operator=(BlankPage&& that) = default;
		~BlankPage();
		[[idlgen::property]]
		hstring UnqualifiedType();
		[[idlgen::property]]
		winrt::hstring Property();
		[[idlgen::property]]
		void Property(winrt::hstring const& a);
		[[idlgen::property]]
		void VoidGetterIsMethod();
		event_token Event(winrt::Windows::Foundation::EventHandler<int32_t> const& handler);
		void Event(winrt::event_token token);
		event_token EventWithConstRefToken(winrt::Windows::Foundation::EventHandler<int32_t> const& handler);
		void EventWithConstRefToken(winrt::event_token const& token);
		winrt::event_token TypedEvent(winrt::Windows::Foundation::TypedEventHandler<BlankPage, uint32_t> const& handler);
		winrt::event_token TypedIncludeEvent(winrt::Windows::Foundation::TypedEventHandler<Root::A::TestIncludeInTemplate, uint32_t> const& handler);
		winrt::event_token TestIncludeDelegate(Root::SomeEventHandler const& handler);
		void TypedEvent(winrt::event_token token);
		void NoSetterOnlyProperty(bool a);
		[[idlgen::property]]
		Category Enum();
		[[idlgen::property]]
		Windows::Foundation::Numerics::Vector2 Struct();
		[[idlgen::property]]
		bool Getter();
		[[idlgen::property]]
		bool const& ConstRefGetter();
		Root::SomeEnum AuthoredEnum();
		Root::SomeStruct AuthoredStruct();
		Root::SomeInterface AuthoredInterface();
		uint32_t MethodPropertyLike();
		void MethodPropertyLike(uint32_t a);
		void MethodPure();
		void MethodOverriden(uint32_t a) override;
		void MethodEnum(TestEnumInMethod a);
		void MethodConstRefEnum(TestEnumInMethod const& a);
		bool Method(bool a);
		void MethodBool(bool a);
		void MethodConstRefBool(bool const& a);
		void MethodFloat(float a, double b);
		void MethodConstRefFloat(float const& a, double const& b);
		void MethodInt(int a, long b, int16_t c, int32_t d, int64_t e);
		void MethodConstRefInt(int const& a, long const& b, int16_t const& c, int32_t const& d, int64_t const& e);
		void MethodUInt(uint8_t a, uint16_t b, uint32_t c, uint64_t d);
		void MethodConstRefUInt(uint8_t const& a, uint16_t const& b, uint32_t const& c, uint64_t const& d);
		void MethodObject(Windows::Foundation::IInspectable const& a);
		void MethodDateTime(Windows::Foundation::DateTime const& a);
		void MethodTimeSpan(Windows::Foundation::TimeSpan const& a);
		void MethodConst(winrt::hstring const a);
		void MethodIncomplete(IncompleteViewModel const&);
		void MethodOverload(bool a);
		void MethodOverload(uint64_t a);
		void MethodOverload(bool a, uint64_t b);
		void MethodOverload(uint64_t a, bool b);
		winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::Foundation::TypedEventHandler<uint32_t, uint32_t>> MethodTemplateInTemplate();
		void NamespaceSame(Root::A::SameViewModel const& a);
		void NamespaceShallower(Root::ShallowerViewModel const& a);
		void NamespaceSibling(Root::B::SiblingViewModel const& a);
		void DifferentPath(Root::SomeNamespace::DifferentPathViewModel const& a);
		[[idlgen::property]]
		static Windows::UI::Xaml::DependencyProperty DependencyProperty();
		[[idlgen::property]]
		SameViewModel ReturnAllowImpl();
		[[idlgen::property]]
		SameViewModel ImplPropertyOnlyExposeGetter();
		[[idlgen::property]]
		void ImplPropertyOnlyExposeGetter(SameViewModel const& a);
		[[idlgen::property]]
		TestIncludeImpl TestIncludeImplWithOnlyImplUse();
		ImplStruct InternalMethod();
		void InternalMethod(ImplStruct const& s);
		void ParamDisallowImpl(SameViewModel const& a);
		SameViewModel ParamDisallowImplEvenReturnAllow(SameViewModel const& a);
		void MethodMixingImplAndProjected(Root::A::SameViewModel const& a, Root::A::implementation::SameViewModel const& b);
		[[idlgen::hide]]
		void HideMethod();
		[[idlgen::protected]]
		void AttrProtectedMethod();
		[[idlgen::overridable]]
		void AttrOverridableMethod();
		virtual void OverridableMethod();
		[[idlgen::property]]
		virtual bool OverridableGetter();
		[[idlgen::property]]
		virtual void OverridableProp(bool a);
		[[idlgen::property]]
		virtual bool OverridableProp();
		winrt::com_array<int32_t> Array();
		void Array(winrt::array_view<int32_t const> a);
		void ArrayRef(winrt::array_view<int32_t> a);
		void ArrayOut(winrt::com_array<int32_t> a);
		[[idlgen::property]]
		void ArrayProp(winrt::array_view<int32_t> a);
		[[idlgen::property]]
		winrt::com_array<int32_t> ArrayProp();
		[[idlgen::property]]
		cppxaml::XamlProperty<bool> CppXamlProperty;
		// Don't need property attribute here or at class level
		wil::single_threaded_property<Root::A::SameViewModel> WilProp;
		wil::single_threaded_rw_property<Root::A::SameViewModel> WilRwProp;
		[[idlgen::property]]
		[[idlgen::overridable]]
		cppxaml::XamlProperty<bool> OverridableCppXamlProperty;
		[[idlgen::overridable]]
		wil::single_threaded_property<Root::A::SameViewModel> OverridableWilProp;
		[[idlgen::overridable]]
		wil::single_threaded_rw_property<Root::A::SameViewModel> OverridableWilRwProp;
		wil::simple_event<int32_t> WilEvent;
		wil::typed_event<int32_t, int32_t> WilTypedEvent;
	protected:
		[[idlgen::hide]]
		void HideProtectedMethod();
		void ProtecedMethod();
		[[idlgen::property]]
		bool ProtecedGetter();
		[[idlgen::property]]
		uint32_t ProtecedProp();
		[[idlgen::property]]
		void ProtecedProp(uint32_t a);
		[[idlgen::property]]
		cppxaml::XamlProperty<bool> ProtectedCppXamlProperty;
		wil::single_threaded_property<Root::A::SameViewModel> ProtectedWilProp;
		wil::single_threaded_rw_property<Root::A::SameViewModel> ProtectedWilRwProp;
		static void ProtecedStaticMethod();
		[[idlgen::property]]
		static cppxaml::XamlProperty<bool> ProtectedStaticCppXamlProperty;
		static wil::single_threaded_property<Root::A::SameViewModel> ProtectedStaticWilProp;
		static wil::single_threaded_rw_property<Root::A::SameViewModel> ProtectedStaticWilRwProp;
	private:
		void PrivateMethod();
	};
}

namespace winrt::Root::A::factory_implementation
{
	struct BlankPage : BlankPageT<BlankPage, implementation::BlankPage>
	{
	};
}
