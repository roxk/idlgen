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

For midl/WinRT construct that are expressible/orthogonal in C++, idlgen does not invent any new types/patterns. The following tables list such construct and how they are expressed in C++ with idlgen:

|midl/WinRT construct|C++|Example in midl|Example in C++|
|--|--|
|WinRT namespace|a designated nested namespace pattern|`namespace MyNamespace`|`namespace winrt::MyNamespace::implementation::idlgen`|
|Runtime class|struct/class in implementation::idlgen|`runtimeclass DependencyObject`|`struct DependencyObject`|
|Constructor (factory)|Constructor|`DependencyObject()`|`DependencyObject()`|
|Method|Member function|`Int32 Func()`|`uint32_t Func()`|
|Primitive types|C++ primitive types|`Double`|`double`|
|Import|Include|`import "A.idl"`|`include "A.h"`|
|Property|`wil`'s property helper|`Int32 MyProperty{get;set;};`|`wil::single_threaded_rw_property<int32_t>`|

### Concepts Not Expressible in C++

For concepts not expressible in C++, the following table list how they are expressed in C++ with idlgen:

|midl/WinRT component authoring concept|C++ concepts|
|--|--|
|Private|C++ private|
|Attribute|`idlgen::attribute` attribute|

### Concepts expressible in C++, But Cause Ambiguous Name Lookup

#### Inheritance

C++ supports inheritance, but inheriting a projected type in C++/WinRT directly is not supported.

Specifying inheritance via `winrt::implement<BaseType>` in an implementation type is also not supported as cppwinrt expects the reverse - the flow should be declaring inheritance in midl, and then generating all the interface/compose types according to the metadata. 

It is also difficult to educate users what the difference between `struct Class : ClassT<Class, Interface>` and `runtimeclass Class : Interface` is.

**Solution**: Custom marker template type. The marker type template signifies to idlgen the base class and interfaces.

#### Import of Types Without Implementation

C++ headers can include `<winrt/RootNamespace.h>` and recognize all such types, but the information of where the types is defined is lost due to how C++WinRT bundles all of the enum/struct/delegate types together in a single header file.

**Solution**: `idlgen::import` attribute. 

*Note*:With support for generating enum/struct/delegate, it is now actually possible to generate all import without attributes if developers include all implementation types or authoring marker types's header.

#### Enum/Struct/Delegate Declaration

C++ has concepts for enum and struct, but not for delegate. Regardless, due to how C++/WinRT works, we can't simply define an enum/struct in C++ and use it as the source for generating an enum in idl. This is because there is no way to tell C++/WinRT to skip generating enum/struct/delegate, and without such abilities any attempt to use the enum/structs/delegate directly would cause conflict.

Suppose developers declared `enum A` in a header, and that idlgen recognizes it and generate the declaration in idl. C++/WinRT would read in the idl and generate an exact same `enum A` in the same namespace. Any use of `A` would cause a hard error of re-definition.

**Solution**: Implementation enum/struct/delegate. Just like when authoring a runtimeclass, there is a distiction between projected/implementation type, enum/struct/delegate can also have this projected/implementation distiction.

Specifically, any enum/struct/delegate declared with a base `idlgen::author_*` marker type is considered an _implementation_ enum/struct/delegate. idlgen would recognize such marker type and generate the corresponding declaration in idl. Developers are exepcted to use the _projected_ enum/struct/delegate throughout their code.

## FAQ

**Why is a C++ method a runtime class method by default? Why doesn't idlgen intelligently distinguish between method and properties?**

This design has been pursued and feedback indicates that the behavior of treating some methods as properties and some other methods, methods, is surprising.

It has been argued that getter/setter is likely more prevalent (e.g. see XAML classes), but ultimately, robustness win over convenience. Conceptually, a C++ method is indeed just a method. Not assuming it can become property reduces cognitive overhead.

For ergonomics, the _property helper_ pattern (#12) should greatly reduce boilerplate by automatically recognizing _configured_ types as property. There is currently no such pattern available for simplifying boilerplate for a method declaration.

TLDR: With #12, method-by-default is a win-win. It's robust by default, and extensible configurably.
