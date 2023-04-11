# Design

This document is inteded for developers who are interested in knowing how idlgen works.

## Overview

The challenge of generating idl from C++ (or really, any other languages) is two folds. First, some of [midl 3.0](https://learn.microsoft.com/en-us/uwp/midl-3/intro) is a superset of C++. That is, there are concepts in midl that are not expressable in C++ via vanilla C++. Second, parsing C++ itself.

idlgen gets away with the first problem by inventing its own DSL on top of vanilla C++. This is inevitable - we need some ways to express concepts that are not inherit to C++. This DSL manifests itself in idlgen as some marker types, and some implementation-defined attributes. This is not the only possible design - but idlgen tries to achieve the following while designing the DSL:

1. Code generation should be robust
2. The DSL should be ergonomic (i.e. as little boilerplate as possible)
3. The DSL should be type-safe
4. The DSL's only prerequisite is C++/WinRT
5. The DSL is extensible via C++'s own features

For the second problem, idlgen turns to clang. idlgen is actually parsing C++ header files with full C++ grammar. This is also inevitable. Inventing its own parser and grammar is only going to increase developer's cognitive load. The library also wants to stick to Microsoft's recent commitment to standard comformance - that is, the library encourange developers keep writing standard-conforming C++. Any DSL the library invents has to be expressible in terms of C++ features.

## Design Details

For midl concepts, and C++/WinRT component authoring concepts that are expressible/orthogonal in C++, idlgen does not invent any new types/patterns. The following tables list such concepts and how they are expressed in C++ with idlgen:

|midl/WinRT component authoring concept|C++ concepts|
|--|--|
|WinRT namespace|C++ namespace, excluding implementation specific details|
|Runtime class|C++ projected type|
|Implementation namespace and type|Type `ClassT` and `implementation` namespace|
|Constructor (factory)|C++ constructor|
|Method|C++ method|
|Primitive types|C++ primitive types|
|Import|Include*|

[*]: For header containing implementation type, or authoring types recognized by idlgen

### Concepts Not Expressible in C++

For concepts not expressible in C++, the following table list how they are expressed in C++ with idlgen:

|midl/WinRT component authoring concept|C++ concepts|
|--|--|
|Property|`idlgen::property` attribute|
|Private/Override methods|C++ private, or `idlgen::hide` attribute|
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
