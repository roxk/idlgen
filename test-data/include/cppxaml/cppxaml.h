#pragma once

// Copied directly from
// https://github.com/asklar/xaml-islands/blob/bbee7bc3c1d67e586f63a5e03279a6986c5ce6f1/inc/cppxaml/XamlProperty.h#L127
namespace cppxaml
{
    template<typename T>
    struct XamlProperty {
        using Type = T;
        T operator()() const {
            return m_value;
        }
        void operator()(const T& value) {
            m_value = value;
        }

        operator T() {
            return m_value;
        }

        XamlProperty<T>& operator=(const T& t) {
            operator()(t);
            return *this;
        }

        template<typename... TArgs>
        XamlProperty(TArgs&&... args) : m_value(std::forward<TArgs>(args)...) {}

    private:
        Type m_value{};
    };
}