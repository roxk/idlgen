# How Getter and Property Templates Work

C++/WinRT's convention for getter is `T Property()`, and setter `void Property(T)`. To implement one simple property, developers need to define at least 2 methods and 1 data member.

## Property Helper Pattern

C++'s method invocation syntax works for all callable. That is to say, the following is legal C++:
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
Cat().Meow();
```

Therefore, a non-static data member which is callable also satisfy C++/WinRT's property convention.

To take advantage of this syntax, property helper classes were developed, e.g. [cppxaml::XamlProperty](https://github.com/asklar/xaml-islands/blob/main/inc/cppxaml/XamlProperty.h).

Idlgen recognizes non-static data member whose type overloads `operator()`. By default, these `operator()` are treated as methods. By configuring these template types as setter or property, idlgen can treat these `operator()` as getter or property instead.
