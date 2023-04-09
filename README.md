# idlgen, an IDL Generator for WinRT

[![nuget](https://img.shields.io/nuget/v/IdlGen.IdlGen.Cpp)](https://www.nuget.org/packages/IdlGen.IdlGen.Cpp/)
[![CI](https://github.com/roxk/idlgen/actions/workflows/ci.yaml/badge.svg)](https://github.com/roxk/idlgen/actions/workflows/ci.yaml)

A library for generating idl files when implementing WinRT components. Currently, only C++ is supported.

## Installation

Use your preferred way (cli/GUI) to install the nuget package `IdlGen.IdlGen.Cpp`, which can be found on [nuget gallery](https://www.nuget.org/packages/IdlGen.IdlGen.Cpp/).

## Usage (C++)

1. Make sure your project can compile and build.
2. Add `pch.h` and `idlgen.h`, and other necessary includes in the header of your implementation type.
3. Edit a header file of your implementation type.
4. Build the project. A custom build step would run before compilation.
5. Viola! The idl file of the implementation type has been generated.

The library would generate the whole runtimm class definition for you. There should be literally zero edit you need to make on the resultant idl file.

### Automatically Generated Structures

The library could automatically generate the following structures in an idl files:
- Runtime class definition, if an implementation type is found
- All (static) methods, events
- Import for another implementation type.
  - Suppose `A.h` includes `B.h`, and `B.h` contains the definition of the implementation type of `B`, any reference to `B` in `A.h` (projected or implementation), would cause `A.idl` to import `B.idl`.

### Structures Requiring Author Help

#### Base Class and Interfaces

To tell the library that a runtime class inherit some base types, make the implementation type inherit the `idlgen::base<B, I...>` marker template type. Then, specify the project type's base class via template parameter `B`, and interfaces via `I`.

#### Others

1. Attributes
2. Properties
3. Import file of projected types, structs, and enum referenced in the runtime class

The above structures require the help of class author to generate. The library defined a set of custom attributes, which, when declared on a class or methods, would allow the library to generate the missing pieces.

Please see [Idlgen Custom Attributes](#Idlgen-Custom-Attributes) for more details.

### Structures Not Generated

- Interface
- delegate
- struct
- enum. 
- Or anything thing that doesn't need an implementation in C++

Please define them in idl directly. They don't need any implementation in the first place.

### Idlgen Custom Attributes

The syntax for idlgen's custom attributes is
```
[[clang::annotate("idlgen::$attribute=$args")]]
```

The library currently utilize clang's annotate attribute to specify an inner attribute, following C++'s `namespace::attribute(args)` convention. `=` is used instead of `()` to simplify parsing.

Below is a table for all attributes and their usage.

|Attribute|Args|Description|Applicable On|Example Declaration|Resultant idl|
|--|--|--|--|--|--|
|`import`|`value,value,...`|Add import statement(s)|class|`[[clang::annotate("idlgen::import=A.idl,B.idl"]]`|`import "A.idl";import "B.idl";`|
|`attribute`|`value`|Add an attribute|class|`[[clang::annotate("idlgen::attribute=default_interface")]]`|`[default_interface]`|
|`getter`|N/A|The method is a getter|method|`[[clang::annotate("idlgen::getter")]]`||
|`setter`|N/A|The method is a setter|method|`[[clang::annotate("idlgen::setter")]]`||
|`hide`|N/A|Hide class or methods|class/method|`[[clang::annotate("idlgen::hide")]]`||

## Tips

### Header Modification

|What to achieve in IDL|What to do in header|
|--|--|
|Hide public event handler in XAML|Make the handler private, set `friend struct ClassT<Class>`|
|Hide public event handler for any other runtime class|Make the handler private, add `friend ClassT<Class>`|
|Hide public overriden methods|Use `[[clang::annotate("hide")]]` on the method|

Note: The friend syntax is different because for XAML `ClassT` is a struct, for any other runtime class it is an alias template.

### Generate IDL for Only One Header

Run `msbuild -target:IdlGenCppGenerateIDL -p:IdlGenCppInclude=MyClass.h -p:Platform=x64`.

If `MyClass.h` is excluded globally in property page, add `-p:IdlGenCppExclude=""` to the command.

## Build Property

|Name|Description|
|--|--|
|`IdlGenCppGenerateIDL`|Master control to configure whether idl files are generated|
|`IdlGenCppInclude`|Control which and only files to include|
|`IdlGenCppExclude`|Exclude the files from generating idl|
|`IdlGenCppDisableUnknownAttributeWarning`|Disable unknown attribute warning|

## Troubleshooting

#### The library failed to generate idl file for my implementation type

The tool needs to parse the header file as if it's compiling it, so _all_ necessary headers have to be included. Make sure you add all necessary includes in the header file. It's suggested that you add `pch.h` to all header files to ease include effort. There should be no additional compilation penalty.

#### I made a mistake in the header file, and the library generated an idl file that cannot be compiled! Now the project is stuck at failing to build idl and cannot re-generate the idl...

Relax. First, copy the old content from `.idl.bak` generated by the library so that the build system can finish the idl building phase. Next, fix your header file.

If you are in the heat of refactoring a class in a header file and do not want the tool to generate idl at all, you can add the header file to property `IdlGenCppExclude`, either manually or via VS's project property page.

## Incremental Adoption in Existing Codebase

In a large code base, it is difficult to edit all existing header files to make them generate correct idl. It is suggested that large code base strictly follows the edit-one-header-at-a-time workflow to avoid generating a bunch of invalid idl files which stop the project from building.

Follow these steps to configure the project for an edit-one-header-at-a-time workflow:
1. Set `IdlGenCppGenerateIDL` to true.
2. Set `IdlGenCppExclude` to `**/*.h` to exclude all headers by default.
3. Follow the instruction in [Generate IDL for Only One Header](#Generate-IDL-for-Only-One-Header) to generate only one header file at at time.

## CI Consideration

Since idl files are meant to be included in the source, there is no need to generate these idl files in CI (Continuous Integration). The library provide an option to control generating idl files (`IdlGenCppGenerateIDL` for C++). .Make sure you set this property to `false` in CI.

## Dev (Inner) Loop Consideration

1. Edit as few headers, ideally only one header, at a time with clean commit
2. If something goes wrong, copy the idl from .bak directly to start over

## Contribution

Contributions are welcome! If you found a bug, please file a bug report.

For more details, such as build instruction, please see [Contribution.md](Contribution.md).
