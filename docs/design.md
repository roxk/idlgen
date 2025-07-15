# Design

This document is inteded for developers who are interested in knowing how idlgen works.

## Overview

The challenge of generating idl from C++ (or really, any other languages) is two folds. First, some of [midl 3.0](https://learn.microsoft.com/en-us/uwp/midl-3/intro) is a superset of C++. That is, there are concepts in midl that are not expressable in C++ via vanilla C++. Second, parsing C++ itself.

idlgen solves the first problem by inventing its own DSL on top of vanilla C++. We need _some_ ways to express concepts that are not expressible in plain C++. The amount of extra DSL to learn to author WinRT types decide the DX of idlgen. Idlgen 1.0 wasn't very good in this regard. Idlgen 2.0 is much better.

In idlgen 1.0, the design aims to keep the existing C++/WinRT header structure, and the DSL manifests itself as some marker types and implementation-defined attributes. There are also certain rules to remember when authoring protected/private members/handlers. You need to configure IdlgenCpp build settings for property accessor.

While idl generation technically works, the experience isn't very good. There were obvious improvements to be made (support macro, alias), but it was obvious that this direction wouldn't get us very far as the major bottleneck was parsing arbitrary code in implementation type's header. That is, idlgen is only interested in WinRT type definition, but it has to parse a lots of irrelevant code as the implementation type naturally mixes lots of ordinary C++ code (std, the generated headers, etc).

Idlgen 2.0 flips the problem around by limiting what developers can write when trying to author generated WinRT types. Thanks to certain limitation, idlgen can generate idl from C++ headers fast, and with much better ergonomics. These limitations include:
- authored WinRT types must be inside a designated nested namespace
- no template is allowed
- property are expressed using designated property accessor
- event are similarly expressed using designated event accessor

For the second problem, idlgen 1.0 turns to clang. For 2.0 as of C++26 - it still has to rely on clang, sadly.

Idlgen 1.0 would parse C++ headers using clang to gather authored WinRT types. Then, it would emit idl and deals with bootstrapping and other complexities caused by mixing idl generation source and idl-using code in the same entity.

C++26 gives us static reflection, which contains enough facilities to reflect almost all of authored WinRT types as seen in [this PoC on compiler explorer](https://godbolt.org/z/9M6qKqGvv). However, there is one crucial missing pieces - constexpr IO. We cannot generate idl nor winmd during compilation.

That being said, it is possible to gather the generated idl/winmd as a program output. So in future iteration (idlegen 2.5?), it is possible to replace clang with just ordinary C\+\+26 static reflection code. I envision the replacement to be a program compiled using developer's code and output the generated idl/winmd.

For now, idlgen 2.0 works basically the same way as 1.0 with regards to code parsing.

## Design Details

For midl/WinRT concepts that are expressible/orthogonal in C++, idlgen does not invent any new types/patterns. The following tables list such concepts and how they are expressed in C++ with idlgen:

|midl/WinRT concepts|C++ concepts|Example in midl|Example in C++|
|--|--|
|Namespace|Designated nested namespace|`namespace MyNamespace`|`namespace winrt::MyNamespace::implementation::idlgen`|
|Import|Include|`import "A.idl"`|`include "A.h"`|
|Primitive types|C++ primitive types|`Double`|`double`|
|Runtime class|`class` in implementation::idlgen|`runtimeclass DependencyObject`|`class DependencyObject final`|
|Sealed class|`final` class|`unsealed runtimeclass Foo`|`class Foo`|
|Base class|Base class|`runtimeclass ViewModel : DependencyObject`|`class ViewModel : DependencyObject`|
|Implemented interface|base class of interface|`runtimeclass ViewModel : INotifyPropertyChanged`|`class ViewModel : INotifyProeprtyChanged`|
|Constructor (factory)|Constructor|`DependencyObject()`|`DependencyObject()`|
|Property|`wil`'s property helper|`Int32 MyProperty{get;set;};`|`wil::single_threaded_rw_property<int32_t>`|
|Method|Member function|`Int32 Func()`|`uint32_t Func()`|
|Method overload|Member function overload|`void Func();void Func(Double x);`|`void Func(); void Func(Double x);`|
|`out` parameter|Non-const reference parameter|`void Func(out Double x);`|`void Func(Double& x);`|
|`ref` struct|Not implemented|N/A|N/A|
|Static method|Static member function|`static void Func()`|`static void Func()`|
|Overridable method|Virtual member function|`overridable void Func()`|`virtual void Func()`|
|Protected method|Protected member function|`protected void Func()`|`protected: void Func()`|
|Event|wil's event helper|`event TypedEventHandler<ViewModel, Int32> Event;`|`wil::typed_event<ViewModel, uint32_t> Event`|
|Delegate|Class Overloading `operator()()`|`delegate void Handler(ViewModel sender, Int32 args)`|`class Handler { operator()(ViewModel const& sender, int32_t arg); }`|
|Struct|`struct`|`struct Point { Int32 x; }`|`struct Point { int32_t x; }`|
|Interface|`Class` with only pure virtual function|`interface IInterface { void Func(); }`|`class IInterface { virtual void Func() = 0; }`|
|Enum|`enum class`|`enum Color { Red, Green };`|`enum class Color { Red, Green };`|
|Flags enum|`enum class` extending `idlgen::flags`|`[flags] enum Permissions { None = 0x0, Camera = 0x1 };`|`enum class Permissions : idlgen::flags { None = 0x0, Camera = 0x1 };`|
|Attribute|`idlgen::attribute` attribute|`[bindable] runtimeclass ViewModel`|`[[idlgen::attribute("bindable")]] class ViewModel`|

## FAQ

**Why is a C++ member function considered a runtime class method by default? Why doesn't idlgen intelligently distinguish between method and properties?**

This design has been pursued and feedback indicates that the behavior of treating some methods as properties and some other methods, methods, is surprising.

It has been argued that getter/setter is likely more prevalent (e.g. see XAML classes), but ultimately, robustness win over convenience. Conceptually, a C++ member function is indeed just a method.

Idlgen 2.0 moves to always define property via a predefined property type. No ambiguity, no boilerplate, no attribute, no setup, it just works™️.
