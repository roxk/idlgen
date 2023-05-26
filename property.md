# How Getter and Property Templates Work

C++/WinRT's convention for getter is `T Property()`, and setter `void Property(T)`. To implement one simple property, developers need to define at least 2 methods and 1 data member. The _Property helper pattern_ has been developed to reduce such boilerplate.

## Property Helper Pattern

Property helper pattern works by taking advangage of the fact that, in C++, function call syntax `foo()` works for all _callable_. 

That is to say, the following is legal C++:
```
struct Cat
{
  void Meow() {}
}
Cat().Meow();
```
And so is the following:
```
struct Meower
{
  void operator()() {} // Overloads operator(), making *this callable with ()
}
struct Cat
{
  Meower Meow;
}
Cat().Meow(); // Calls Meower's overloaded operator()
```

IOW, other than `T Property()` and `void Property(T)`, a non-static data member whose type overloads `T operator()` and `void operator(T)` also satisfy C++/WinRT's property convention.

To take advantage of this syntax, property helper classes were developed, e.g. [cppxaml::XamlProperty](https://github.com/asklar/xaml-islands/blob/main/inc/cppxaml/XamlProperty.h).

Idlgen recognizes data members whose type is a template and overloads `operator()` with function signatures matching getter and setter. By default, these `operator()` are treated as methods (that is why `cppxaml::XamlEvent` works automatically). By adding `[[idlgen::property]]` to such data members, or by configuring these templates as getters or properties in project properties, idlgen can treat these overloaded `operator()` as getters or setters.
