# idlgen Cheatsheet

This document omits `#include <winrt/author/base.h>` at the top for brevity. This document also assumes relevant namespaces are included in pch.h or at the top of the file.

## Interface

## Runtimeclass

### Declare a bare runtime class

```
// Note: Must be in author namespace!
namespace winrt::Contoso::author
{
    struct BookSku : winrt::author::runtimeclass<>
    {
    };
}
```

**Note**: All subsequent code snippets assume the class is declared in the `winrt::Contoso::author` namespace, so the namespace declaration is omitted for brevity.

### Declare a XAML page

```
struct MainPage : winrt::author::runtimeclass<winrt::Windows::UI::Xaml::Controls::Page>
{
};
```

### Override projected type methods
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
    winrt::author:setter MyProperty(int32_t value);	// OK: int Property {get; set;};

    int Getter(winrt::author::getter = {});			// OK: int Getter {get;};

    winrt::author::setter Setter(int32_t value);	// ERROR: setter only property is not allowed
};
```

### Add methods to runtimeclass

```
struct WithMethods : winrt::author::runtimeclass<>
{
    void SomeMethod(int integer);
    void SomeMethod(int integer, float floatingPoint);  // OK: method overloading with different arity is supported
    void SomeMethod(float floatingPoint);               // ERROR: Overload with the same arity not allowed (behavior is undefined)
    void SomeMethodFp(float floatingPoint);             // OK: Give your methods unique names to avoid confusion with overloads if they have the same arity but differ only by parameter types
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

### Implement projected interface
```
struct WithProjected : winrt::author::runtimeclass<winrt::Windows::Foundation::IStringable>
{
    winrt::hstring ToString(winrt::author::override = {});  // Ok: specify override
    winrt::hstring ToString();                              // Error: Ambiguous method call
};
```

### Implement internal interface (i.e. the fact that this class implements the interface is NOT exposed in winmd)
```
struct WithInternal : winrt::author::runtimeclass<winrt::author::internal<winrt::Windows::Foundation::IStringable>>
{
    winrt::hstring ToString(winrt::author::override = {});  // Ok: specify override
    winrt::hstring ToString();                              // Error: Ambiguous method call
};

// resultant idl:
runtimeclass WithInternal
{
}
```

### Unsealed Class (Composable Class)

```
struct ComposableClass : winrt::author::runtimeclass<>, winrt::author::unsealed
{
};
```

### Inherit from authored runtimeclass (the base must be unsealed)
```
struct Base : winrt::author::runtimeclass<>
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

struct Base : winrt::author::runtimeclass<>
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
```
struct Base : winrt::author::runtimeclass<>
{
    virtual int Value(winrt::author::getter = {});
};
struct Derived : winrt::author::runtimeclass<Base>
{
    int Value(winrt::author::override = {}) override;
};
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

struct MultiInterface : winrt::author::runtimeclass<>, IControl, IListBox
{
    void Paint() override;
    void SetItems(winrt::array_view<winrt::hstring> items) override;
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

## Struct
