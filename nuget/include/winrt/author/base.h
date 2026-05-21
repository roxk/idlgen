#pragma once
#include <winrt/base.h>
#include <type_traits>

namespace winrt::author
{
    struct winrt_interface
    {};
    template <typename... Base>
    struct runtimeclass
    {
        virtual void use_make_function_to_create_this_object() = 0;
    };
    template <>
    struct runtimeclass<>
    {
        virtual void use_make_function_to_create_this_object() = 0;
    };
    struct unsealed
    {};
    struct setter
    {};
    struct getter
    {};
    struct ignore
    {};
    struct static_class
    {};
    struct partial
    {};
    struct delegate
    {};
    struct winrt_struct
    {};
    struct attribute_target
    {
    protected:

    private:
        friend struct target_all;
        friend struct target_delegate;
        friend struct target_enum;
        friend struct target_event;
        friend struct target_field;
        friend struct target_interface;
        friend struct target_method;
        friend struct target_parameter;
        friend struct target_property;
        friend struct target_runtimeclass;
        friend struct target_struct;
        attribute_target() = default;
    };
    struct target_all : attribute_target
    {};
    struct target_delegate : attribute_target
    {};
    struct target_enum : attribute_target
    {};
    struct target_event : attribute_target
    {};
    struct target_field : attribute_target
    {};
    struct target_interface : attribute_target
    {};
    struct target_method : attribute_target
    {};
    struct target_parameter : attribute_target
    {};
    struct target_property : attribute_target
    {};
    struct target_runtimeclass : attribute_target
    {};
    struct target_struct : attribute_target
    {};
    template<typename T>
    concept AttributeTarget = std::is_base_of_v<winrt::author::attribute_target, T>;
    template<AttributeTarget... Target>
    struct attributeusage
    {};
    struct allowmultiple
    {};
    // TODO: Constraint parameters to be only attributeusage or allowmultiple
    template<typename... T>
    struct attribute
    {};
    // TODO: Constraint first param to be an attribute
    template<typename T, auto... Args>
    struct apply_attr
    {};
    template <typename T, size_t N>
    struct attr_string
    {
        T data[N];
        constexpr attr_string(T const (&s)[N])
        {
            std::copy_n(s, N, data);
        }
    };
    template <typename T, size_t N>
    struct attr_type : attr_string<T, N>
    {
        using attr_string<T, N>::attr_string;
    };
    struct allowforweb
    {};
    struct contentproperty
    {};
    struct contract
    {};
    struct uuid
    {};
    struct interface_name
    {};
}
