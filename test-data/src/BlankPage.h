#pragma once

#include <cstdint>
#include "idlgen.h"
// Test inconsistent file name case
#include "blankpage.g.h"
#include "SameViewModel.h"
#include "TestIncludeImpl.h"
#include "TestIncludeInTemplate.h"

namespace winrt::Root::A::implementation
{
	// Make sure we don't crash when checking against template parameter (not all args are type)
	template <typename... T>
	struct PackedTemplate {};

	template <int I>
	struct IntegerTemplate {};

	struct ImplStruct : PackedTemplate<ImplStruct>, IntegerTemplate<10> {};

	struct
		[[clang::annotate("idlgen::import=SameViewModel.idl,ShallowerViewModel.idl,"
			"SiblingViewModel.idl")]]
	[[clang::annotate("idlgen::attribute=bindable")]]
	[[clang::annotate("idlgen::attribute=default_interface")]]
	BlankPage : BlankPageT<BlankPage>, idlgen::base<Windows::UI::Xaml::Controls::Page, Windows::UI::Xaml::Data::INotifyPropertyChanged>
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
		[[clang::annotate("idlgen::property")]]
		hstring UnqualifiedType();
		[[clang::annotate("idlgen::property")]]
		winrt::hstring Property();
		[[clang::annotate("idlgen::property")]]
		void Property(winrt::hstring const& a);
		[[clang::annotate("idlgen::property")]]
		void VoidGetterIsGeneratedAsIs();
		event_token Event(winrt::Windows::Foundation::EventHandler const& handler);
		void Event(winrt::event_token token);
		event_token EventWithConstRefToken(winrt::Windows::Foundation::EventHandler const& handler);
		void EventWithConstRefToken(winrt::event_token const& token);
		winrt::event_token TypedEvent(winrt::Windows::Foundation::TypedEventHandler<BlankPage, uint32_t> const& handler);
		winrt::event_token TypedIncludeEvent(winrt::Windows::Foundation::TypedEventHandler<Root::A::TestIncludeInTemplate, uint32_t> const& handler);
		void TypedEvent(winrt::event_token token);
		void NoSetterOnlyProperty(bool a);
		[[clang::annotate("idlgen::property")]]
		Category Enum();
		[[clang::annotate("idlgen::property")]]
		Windows::Foundation::Numerics::Vector2 Struct();
		[[clang::annotate("idlgen::property")]]
		bool Getter();
		void MethodPure();
		bool Method(bool a);
		void MethodBool(bool a);
		void MethodFloat(float a, double b);
		void MethodInt(int a, long b, int16_t c, int32_t d, int64_t e);
		void MethodUInt(uint8_t a, uint16_t b, uint32_t c, uint64_t d);
		void MethodObject(Windows::Foundation::IInspectable const& a);
		void MethodDateTime(Windows::Foundation::DateTime const& a);
		void MethodTimeSpan(Windows::Foundation::TimeSpan const& a);
		void MethodConst(winrt::hstring const a);
		void MethodIncomplete(IncompleteViewModel const&);
		void MethodOverload(bool a);
		void MethodOverload(uint64_t a);
		void MethodOverload(bool a, uint64_t b);
		void MethodOverload(uint64_t a, bool b);
		void NamespaceSame(Root::A::SameViewModel const& a);
		void NamespaceShallower(Root::ShallowerViewModel const& a);
		void NamespaceSibling(Root::B::SiblingViewModel const& a);
		[[clang::annotate("idlgen::property")]]
		static Windows::UI::Xaml::DependencyProperty DependencyProperty();
		[[clang::annotate("idlgen::property")]]
		SameViewModel ReturnAllowImpl();
		[[clang::annotate("idlgen::property")]]
		SameViewModel ImplPropertyOnlyExposeGetter();
		[[clang::annotate("idlgen::property")]]
		void ImplPropertyOnlyExposeGetter(SameViewModel const& a);
		[[clang::annotate("idlgen::property")]]
		TestIncludeImpl TestIncludeImplWithOnlyImplUse();
		ImplStruct InternalMethod();
		void InternalMethod(ImplStruct const& s);
		void ParamDisallowImpl(SameViewModel const& a);
		SameViewModel ParamDisallowImplEvenReturnAllow(SameViewModel const& a);
		void MethodMixingImplAndProjected(Root::A::SameViewModel const& a, Root::A::implementation::SameViewModel const& b);
		[[clang::annotate("idlgen::hide")]]
		void HideMethod();
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
