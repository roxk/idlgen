# idlgen Cheatsheet

This document omits `#include <winrt/author/base.h>` at the top for brevity. This document also assumes relevant namespaces are included in pch.h or at the top of the file.

**Note**: All subsequent code snippets assume the class is declared in the `winrt::Contoso::author` namespace for brevity, unless otherwise specified.

**Remember: You must declare you author types in author namespace!**

## Interface

### Declare an interface
```
struct IShape : winrt::author::winrt_interface
{
    virtual void Draw() = 0;
};
```

### Require (extend) an authored interface
```
struct IShape : winrt::author::winrt_interface
{
    virtual void Draw() = 0;
};
struct IColoredShape : IShape
{
    virtual winrt::hstring Color() = 0;
};
```

### Require (extend) a projected interface
```
struct IDebugPrintable : winrt::author::winrt_interface<winrt::Windows::Foundation::IStringable>
{
    virtual void DebugPrint() = 0;
};
```

## Runtimeclass

### Declare a bare runtime class

```
struct BookSku : winrt::author::runtimeclass<>
{
};
```

### Declare a XAML page (Inherit another composable class)

```
struct MainPage : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page>
{
};
```

### Override base methods
```
struct MainPage : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page>
{
    void MeasureOverride(winrt::Windows::Foundation::Size const& availableSize, winrt::author::override = {});
}
```

### Declare a view model
```
struct MainPageViewModel : winrt::author::runtimeclass<>
{
};
```

### Add properties to runtimeclass

```
struct WithProperties : winrt::author::runtimeclass<>
{
    int MyProperty(winrt::author::getter = {});
    winrt::author:setter MyProperty(int32_t value); // OK: property in winmd

    int Getter(winrt::author::getter = {});         // OK: getter in winmd

    winrt::author::setter Setter(int32_t value);    // Error: setter only property is not allowed
};
```

### Add methods to runtimeclass

```
struct WithMethods : winrt::author::runtimeclass<>
{
    void SomeMethod(int integer);
    void SomeMethod(int integer, float floatingPoint);  // OK: method overloading with different arity is supported
    void SomeMethod(float floatingPoint);               // Error: overload with the same arity not allowed (behavior is undefined)
    void SomeMethodFp(float floatingPoint);             // OK: Not overload. Use different method name if arity is the same
};
```

### Add events to runtimeclass

```
namespace winrt::Contoso
{
    struct WithEvents;
}
struct WithEvents : winrt::author::runtimeclass<>
{
    winrt::event_token SomethingHappened(winrt::Windows::Foundation::TypedEventHandler<winrt::Contoso::WithEvents, int> const& handler);
    void SomethingHappened(winrt::event_token token);
};
```

### Reference other WinRT types in method parameters or properties

Only projected types are allowed. The only exceptions are authored enum and authored WinRT structs.

```
namespace winrt::Contoso
{
    struct ViewModel;
}
namespace winrt::Contoso::author
{
    struct Point : winrt::author::winrt_struct
    {
        float X;
        float Y;
    };
    struct ViewModel : winrt::author::runtimeclass<>
    {
    };
    struct MainPage : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page>
    {
        void Method(winrt::Windows::UI::Xaml::UIElement const& element);
        winrt::Windows::Foundation::IInspectable Getter(winrt::author::getter = {});
        void Draw(Point point);                                                         // OK: authored struct is allowed
        void Bind(ViewModel viewModel);                                                 // Error: authored runtimeclass not allowed
        void Bind(winrt::Contoso::ViewModel viewModel);                                 // OK: Contoso::ViewModel is projected type (forward declared)
    }
}
```

### Hide constructor in winmd
```
struct ViewModel : winrt::author::runtimeclass<>
{
    ViewModel(winrt::author::ignore = {});
    ViewModel(int internalOnly, winrt::author::ignore = {});
    ViewModel(int public, float constructor);                   // Only this shows up in winmd
};
```

### Hide method in winmd
```
struct ViewModel : winrt::author::runtimeclass<>
{
    void ImplementationDetail(winrt::author::ignore = {});
    void NotExposed(int details, winrt::author::ignore = {});
    void InternalOnly(int super, float secret, winrt::author::ignore = {});
    void PublicMethod();                                                        // Only this shows up in winmd
}
```

### Implement projected interface
```
struct WithProjected : winrt::author::runtimeclass<winrt::Windows::Foundation::IStringable>
{
    winrt::hstring ToString(winrt::author::override = {});  // OK: specify override
    winrt::hstring ToString();                              // Error: define another method instead of implementing the interface -> ambiguous method call
    winrt::hstring ToString(winrt::author::ignore = {});    // OK: also work - ignored in winmd, but recognized by idlgen
};
```

### Implement internal interface (i.e. the fact that this class implements the interface is NOT exposed in winmd)
```
struct WithInternal : winrt::author::runtimeclass<winrt::author::internal<winrt::Windows::Foundation::IStringable>>
{
    winrt::hstring ToString(winrt::author::override = {});  // OK: specify override
    winrt::hstring ToString();                              // Error:define another method instead of implementing the interface -> ambiguous method call
    winrt::hstring ToString(winrt::author::ignore = {});    // OK: also work - ignored in winmd, but recognized by idlgen
};

// resultant idl:
runtimeclass WithInternal
{
}
```

### Implement authored interface
```
struct IControl : winrt::author::winrt_interface
{
    virtual void Paint() = 0;
};

struct IListBox : winrt::author::winrt_interface
{
    virtual void SetItems(winrt::array_view<winrt::hstring> items) = 0;
};

struct MultiInterface : winrt::author::runtimeclass<IControl, IListBox>
{
    void Paint() override;
    void SetItems(winrt::array_view<winrt::hstring> items) override;
};
```

### Implement internal authored interface
```
namespace winrt::Contoso::author
{
    struct IControl : winrt::author::winrt_interface
    {
        virtual void Paint() = 0;
    };
    struct ImplementingInternalAuthoredInterface : winrt::author::runtimeclass<winrt::author::internal<IControl>>
    {
        void Paint(winrt::author::override = {});
    };
}
```

### Implement authored interface without QI (i.e. not internal), without exposing in winmd

Impossible.

### Unsealed Class (Composable Class)

```
struct ComposableClass : winrt::author::runtimeclass<>, winrt::author::unsealed
{
};
```

### Inherit from authored runtimeclass (the base must be unsealed)
```
struct Base : winrt::author::runtimeclass<>, winrt::author::unsealed
{
    int Value(winrt::author::getter = {});
};
struct Derived : winrt::author::runtimeclass<Base>
{
    int DerivedValue(winrt::author::getter = {});
};
```

### Declare Protected Methods (the base must be unsealed)

```

struct Base : winrt::author::runtimeclass<>, winrt::author::unsealed
{
protected:
    int Value(winrt::author::getter = {});
};
struct Derived : winrt::author::runtimeclass<Base>
{
    // Only Derived and other derived classes can call Value getter
};
```

### Declare Overridable Methods (the base must be unsealed)

**Note**: Overridable methods in WinRT is protected, so only derived class can call them.

```
struct Base : winrt::author::runtimeclass<>
{
    virtual int ValueOverride();
};
struct Derived : winrt::author::runtimeclass<Base>
{
    int ValueOverride(winrt::author::override = {});
};
```

### Static class

```
struct StaticClass : winrt::author::runtimeclass<>, winrt::author::static_class
{
    static int Compute(int x, int y);
};
```

### Partial Class

```
struct PartialClass : winrt::author::runtimeclass<>, winrt::author::partial
{
};
```

## Enum

### Declare an enum

```
enum class Color : int
{
    Red,
    Green,
    Blue
};
```

### Declare a flags enum
```
enum class FileAccess : unsigned int
{
    Read = 0x1,
    Write = 0x2,
    Execute = 0x4
};
```

## Struct

### Declare struct
```
struct Point : winrt::author::winrt_struct
{
    float X;
    float Y;
};
```
