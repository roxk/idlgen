#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>
// #include <winrt/Microsoft.UI.Xaml.Data.h>
#include "winrt/author/base.h"

namespace winrt::Microsoft::UI::Xaml::Input
{
    struct KeyboardAcceleratorInvokedEventArgs;
}

// Forward declare projected type produced from this file
namespace winrt::App1
{
    struct IControl;
    struct ITest;
    struct Point;
    struct AuthorClass;
    struct AsyncActionCompletedHandler;
}

namespace winrt::App1::author
{
    struct IControl : winrt::author::winrt_interface
    {
        virtual void Paint() = 0;
    };
    struct ITest : IControl
    {
        virtual void SetText(winrt::hstring const& value) = 0;
    };
    struct IListBox : IControl
    {
        virtual void SetItems(winrt::array_view<winrt::hstring> items) = 0;
    };
    struct IComboBox : ITest, IListBox
    {
        virtual void SetSelectedIndex(int index) = 0;
    };
    struct Point : winrt::author::winrt_struct
    {
        int x;
        int y;
    };
    struct AuthorClass : winrt::author::unsealed, winrt::author::runtimeclass<
        winrt::Microsoft::UI::Xaml::FrameworkElement,
        winrt::Windows::Foundation::IStringable
        // winrt::Microsoft::UI::Xaml::Data::INotifyPropertyChanged
    >,
        ITest
    {
        enum State
        {
            State1,
            State2
        };
        unsigned char Byte(winrt::author::getter = {});
        short Short(winrt::author::getter = {});
        unsigned short UShort(winrt::author::getter = {});
        int Int(winrt::author::getter = {});
        unsigned int UInt(winrt::author::getter = {});
        long long Long(winrt::author::getter = {});
        unsigned long long ULong(winrt::author::getter = {});
        wchar_t Character(winrt::author::getter = {});
        winrt::hstring String(winrt::author::getter = {});
        float Float(winrt::author::getter = {});
        double Double(winrt::author::getter = {});
        bool Boolean(winrt::author::getter = {});
        winrt::guid GuidType(winrt::author::getter = {});
        winrt::Windows::Foundation::DateTime DateTime(winrt::author::getter = {});
        winrt::Windows::Foundation::TimeSpan TimeSpan(winrt::author::getter = {});
        winrt::Windows::Foundation::Point Point(winrt::author::getter = {});
        winrt::Windows::Foundation::Size Size(winrt::author::getter = {});
        winrt::Windows::Foundation::Rect Rect(winrt::author::getter = {});
        winrt::Windows::Foundation::IInspectable Object(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IIterable<int> Iterable(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IIterator<int> Iterator(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IKeyValuePair<int, int> KeyValuePair(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IMap<int, int> Map(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IMapChangedEventArgs<int> MapChangedEventArgs(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IMapView<int, int> MapView(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IObservableMap<int, int> ObservableMap(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IObservableVector<int> ObservableVector(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IVector<int> Vector(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::IVectorView<int> VectorView(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::MapChangedEventHandler<int, int> MapChangedEventHandler(winrt::author::getter = {});
        // winrt::Windows::Foundation::Collections::VectorChangedEventHandler<int> VectorChangedEventHandler(winrt::author::getter = {});
        winrt::Windows::Foundation::IAsyncAction AsyncAction();
        winrt::Windows::Foundation::IAsyncOperation<int> AsyncOperation();
        void PassArray(winrt::array_view<int const> values);
        void FillArray(winrt::array_view<int> values);
        void ReceiveArray(winrt::com_array<int>& values);
        winrt::com_array<int>& ReturnComArray();
        void PassArrayStruct(winrt::array_view<App1::Point const> values);
        void FillArrayStruct(winrt::array_view<App1::Point> values);
        void ReceiveArrayStruct(winrt::com_array<App1::Point>& values);
        void InputParameterRefConst(winrt::Windows::Foundation::Point const& point);
        void OutParameterStruct(winrt::Windows::Foundation::Point& point);
        winrt::App1::author::Point UseAuthorStruct(winrt::App1::author::Point point);
        void RefConstAuthoredStruct(App1::Point const& point);
        void OutAuthoredStruct(App1::Point& point);
        void OutParameterReferenceType(winrt::Windows::Foundation::IInspectable& object);
        winrt::Windows::Foundation::IReference<int> BoxedInt(winrt::author::getter = {});

        void Ignored(winrt::author::ignore = {});
        void Paint() override;
        void SetText(winrt::hstring const& value) override;
        static void StaticMethod();
        void OnKeyboardAcceleratorInvoked(winrt::Microsoft::UI::Xaml::Input::KeyboardAcceleratorInvokedEventArgs const& args, winrt::author::override = {});
        winrt::hstring ToString(winrt::author::override = {});
    protected:
        void ProtectedMethod();
    public:
        void UsingAuthoredType(winrt::App1::ITest const& test);
        int Foo();
        int Foo(int value);
        int Foo(int value, float value2);
        int Getter(winrt::author::getter = {});
        winrt::author::setter Property(int value);
        int Property(winrt::author::getter = {});

        // static winrt::event_token StaticEvent(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        // static void StaticEvent(winrt::event_token token);
        winrt::event_token UntypedEvent(winrt::Windows::Foundation::EventHandler<int> const& handler);
        void UntypedEvent(winrt::event_token token);
        winrt::event_token TypedEvent(winrt::Windows::Foundation::TypedEventHandler<int, float> const& handler);
        void TypedEvent(winrt::event_token token);
        winrt::event_token SelfReferenceEvent(winrt::Windows::Foundation::TypedEventHandler<App1::AuthorClass, App1::ITest> const& handler);
        void SelfReferenceEvent(winrt::event_token token);
        winrt::event_token AsyncActionCompleted(winrt::Windows::Foundation::AsyncActionCompletedHandler const& handler);
        void AsyncActionCompleted(winrt::event_token token);
        winrt::event_token AsyncActionProgress(winrt::Windows::Foundation::AsyncActionProgressHandler<int> const& handler);
        void AsyncActionProgress(winrt::event_token token);

        AuthorClass();
        AuthorClass(winrt::Windows::Foundation::IInspectable const& object, winrt::author::ignore = {});
    private:
        // winrt::event<winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> mPropertyChanged{};
    };
    struct Area : winrt::author::runtimeclass<>
    {
        Area(int width, int height);
        int Height(winrt::author::getter = {});
        winrt::author::setter Width(int value);
        int Width(winrt::author::getter = {});
        winrt::author::setter Height(int value);
        // TODO: Fix static not showing up
        static int NumberOfAreas(winrt::author::getter = {});
    };
    struct StaticArea : winrt::author::runtimeclass<>, winrt::author::static_class
    {
        static void F();
        static void F(double x);
        static void F(double x, double y);
    };
    struct PartialClass : winrt::author::runtimeclass<>, winrt::author::partial
    {
        static void Divide(int x, int y, int& result, int& remainder);
    };
    struct AsyncActionCompletedHandler : winrt::author::delegate
    {
        AsyncActionCompletedHandler(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::Foundation::AsyncStatus const& args);
    };
    enum class Color : int
    {
        Red,
        Green,
        Blue
    };
    enum class SetOfBooleanValues : unsigned int
    {
        None = 0x00000000,
        Value1 = 0x00000001,
        Value2 = 0x00000002,
        Value3 = 0x00000004,
    };
    struct HelpAttribute : winrt::author::attribute<
        winrt::author::attributeusage<winrt::author::target_runtimeclass, winrt::author::target_property,
        winrt::author::target_method, winrt::author::target_event>, winrt::author::allowmultiple>
    {
        HelpAttribute(winrt::hstring ClassUri, winrt::hstring MemberTopic, int Version);
    };
    struct BookSku : winrt::author::runtimeclass<>, winrt::author::apply_attr<HelpAttribute,
        winrt::author::attr_string("htts://booksku"), winrt::author::attr_string("BookSku class"), 0>,
        winrt::author::unsealed
    {
        virtual void SellOverride();
        virtual void BuyOverride();
        virtual void BuyInColor(Color color);
    };
    struct DerivedBookSku : winrt::author::runtimeclass<BookSku>
	{
        void SellOverride(winrt::author::override = {});
        // Not overriding BuyOverride = buy override adapter should be absent in generated implementation
        void BuyInColor(Color color, winrt::author::override = {});
    };
    struct MyPanel : winrt::author::runtimeclass<
        // winrt::Microsoft::UI::Xaml::Controls::Panel
    >, winrt::author::apply_attr<
        winrt::author::contentproperty, winrt::author::attr_string("Children")>
    {
        winrt::Windows::Foundation::IInspectable Children();
    };
    struct ImplementingInternalInterface : winrt::author::runtimeclass<winrt::author::internal<App1::IControl>>
    {
        void Paint(winrt::author::ignore = {});
    };
    struct Area2 : winrt::author::runtimeclass<
        winrt::Microsoft::UI::Xaml::FrameworkElement,
        winrt::author::internal<winrt::Windows::Foundation::IStringable>
    >
    {
        Area2(int width, int height);
        author::Color Color(winrt::author::getter = {});
        winrt::author::setter Color(author::Color value);
        author::Color GetColor(winrt::author::getter = {});
        void SetValues(SetOfBooleanValues values);
        static void StaticSetValues(SetOfBooleanValues values);
        void MethodWithMixed(author::Color color, SetOfBooleanValues value, winrt::Windows::Foundation::IInspectable const& ref);
        winrt::event_token AsyncActionCompleted(winrt::App1::AsyncActionCompletedHandler const& handler);
        void AsyncActionCompleted(winrt::event_token token);
    protected:
        void ThisShouldNotShowUpInIdl();
    };
    struct Sample : winrt::author::runtimeclass<>,
        winrt::author::apply_attr<winrt::author::interface_name,
        winrt::author::attr_string("IMyName"), winrt::guid("ceb27355-f772-407c-9540-6467a7199bc7")>,
        winrt::author::apply_attr<winrt::author::contract, winrt::author::attr_type("Windows.Foundation.UniversalApiContract"), 1>
    {};
    struct ISomethingMarker : winrt::author::winrt_interface,
        winrt::author::apply_attr<winrt::author::uuid, winrt::author::attr_string("94569FA9-D3BB-4D01-BF7C-B8E1D8F8B30C")>,
        winrt::author::apply_attr<winrt::author::contract, winrt::author::attr_type("Windows.Foundation.UniversalApiContract"), 1>
    {};
}