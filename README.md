# idlgen, an IDL Generator for WinRT

A library for generating idl files when implementing WinRT components. Currently, only C++ is supported.

## Installation

Use your preferred way (cli/GUI) to install the nuget package `IdlGen.IdlGen.Cpp`, which can be found on [nuget gallery](TODO:insert-link).

## Usage (C++)

### Overview

1. Edit the header file of your implementation type.
2. Build the project. A custom build step would run before compilation.
3. Viola! The idl file of the implementation type has been updated.
4. Try deleting the idl file and re-run, you should see the idl file being generated again.

The library would generate the whole runtimm class definition for you. There should be literally zero edit you need to make on the resultant idl file.

The library could automatically generate the following structures in an idl files:
1. Runtime class definition, if an implementation type is found
2. All getters, setters, methods, events
3. Import file for an implementation type, if the implementation type is referenced in the runtime class' properties/methods, etc

The library would not generate the following:
1. Attributes
2. Interfaces and base class
3. Import file of projected types, value types, and enum referenced in the runtime class' properties/methods, etc
4. Structs

To help the tool generate these missing pieces, the library defined the following custom attributes, which, when declared for a class, would cause the library to generate the missing pieces.

### Idlgen Custom Attributes

The syntax for idlgen's custom attributes is
```
[[clang::annotate("idlgen::$attribute=$args")]]
```

The library currently utilize clang's annotate attribute to specify an inner attribute, following C++'s `namespace::attribute(args)` convention. `=` is used instead of `()` to simplify parsing.

Below is a table for all attributes and their usage.

|Attribute|Args|Description|Example Declaration|Resultant idl|
|--|--|--|--|
|`import`|`value,value,...`|Add import statement(s)|`[[clang::annotate("idlgen::import=A.idl,B.idl"]]`|`import "A.idl";import "B.idl";`|
|`attribute`|`value`|Add an attribute|`[[clang::annotate("idlgen::attribute=default_interface")]]`|`[default_interface]`|
|`extend`|`value`|Add base class and interfaces|`[[clang::annotate("idlgen::extend=WUXC.Page,WUXD.INotifyPropertyChanged]]`|`$yourClass : WUXC.Page,WUXD.INotifyPropertyChanged`|

### Other Usages

|What to achieve in IDL|What to do in header|
|--|--|
|Hide method (e.g. handler)|Make the method private, set the class that needs it as friend|
|Hide event handler in XAML|Make the handler private, set `BaseT<Base>` as friend|

## Troubleshooting

#### The library can't generate idl file for my implementation type

The tool needs to parse the header file as if it's compiling it, so _all_ necessary headers have to be included. Make sure you add all necessary includes in the header file. E.g. for a simple `MainPage.h`, you need to add `<windows.h>` and `<winrt/Windows.UI.Xaml.Markup.h>` before `MainPage.g.h`.

#### I made a mistake in the header file, and the library generated an idl file that cannot be compiled! Now the project is stuck at failing to build idl and cannot re-generate the idl...

Relax. Just delete the problematic idl file and rebuild. The idl file would be re-generated after idl compilation finishes. If something really bad happens, you can disable idl generation via (for C++ project) project properties -> C++/IdlGen -> setting Generate IDL to false.

## CI or Dev(Inner)-Loop consideration

Since idl files are meant to be included in the source, there is no need to generate these idl files in CI (Continuous Integration). The library provide an option to control generating idl files (`IdlGenCppGenerateIDL` for C++). .Make sure you set this property to `false` in CI.

Similary, when you are working a part of the project that doesn't need to update idl files, you can set `IdlGenCppGenerateIDL` (for C++) to disable generating idl files.

## Contribution

Contributions are welcome! If you found a bug, please file a bug report.

For more details, such as build instruction, please see [Contribution.md](Contribution.md).
