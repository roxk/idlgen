#pragma once
#include <winrt/base.h>
#include <type_traits>

namespace winrt::author
{
    template <typename... Base>
    struct WINRT_IMPL_EMPTY_BASES winrt_interface
    {};
    template <>
    struct WINRT_IMPL_EMPTY_BASES winrt_interface<>
    {};
    template <typename... Base>
    struct WINRT_IMPL_EMPTY_BASES runtimeclass
    {
        virtual void use_make_function_to_create_this_object() = 0;
    };
    template <>
    struct WINRT_IMPL_EMPTY_BASES runtimeclass<>
    {
        virtual void use_make_function_to_create_this_object() = 0;
    };
    struct WINRT_IMPL_EMPTY_BASES unsealed
    {};
    struct WINRT_IMPL_EMPTY_BASES setter
    {};
    struct WINRT_IMPL_EMPTY_BASES getter
    {};
    struct WINRT_IMPL_EMPTY_BASES ignore
    {};
    struct WINRT_IMPL_EMPTY_BASES static_class
    {};
    struct WINRT_IMPL_EMPTY_BASES partial
    {};
    struct WINRT_IMPL_EMPTY_BASES delegate
    {};
    struct WINRT_IMPL_EMPTY_BASES winrt_struct WINRT_IMPL_EMPTY_BASES
    {};
    struct WINRT_IMPL_EMPTY_BASES attribute_target
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
    struct WINRT_IMPL_EMPTY_BASES target_all : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_delegate : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_enum : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_event : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_field : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_interface : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_method : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_parameter : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_property : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_runtimeclass : attribute_target
    {};
    struct WINRT_IMPL_EMPTY_BASES target_struct : attribute_target
    {};
    template<typename T>
    concept AttributeTarget = std::is_base_of_v<winrt::author::attribute_target, T>;
    template<AttributeTarget... Target>
    struct WINRT_IMPL_EMPTY_BASES attributeusage
    {};
    struct WINRT_IMPL_EMPTY_BASES allowmultiple
    {};
    // TODO: Constraint parameters to be only attributeusage or allowmultiple
    template<typename... T>
    struct WINRT_IMPL_EMPTY_BASES attribute
    {};
    // TODO: Constraint first param to be an attribute
    template<typename T, auto... Args>
    struct WINRT_IMPL_EMPTY_BASES apply_attr
    {};
    template <typename T, size_t N>
    struct WINRT_IMPL_EMPTY_BASES attr_string
    {
        T data[N];
        constexpr attr_string(T const (&s)[N])
        {
            std::copy_n(s, N, data);
        }
    };
    template <typename T, size_t N>
    struct WINRT_IMPL_EMPTY_BASES attr_type : attr_string<T, N>
    {
        using attr_string<T, N>::attr_string;
    };
    struct WINRT_IMPL_EMPTY_BASES allowforweb
    {};
    struct WINRT_IMPL_EMPTY_BASES contentproperty
    {};
    struct WINRT_IMPL_EMPTY_BASES contract
    {};
    struct WINRT_IMPL_EMPTY_BASES uuid
    {};
    struct WINRT_IMPL_EMPTY_BASES interface_name
    {};
    template <typename... T>
    struct WINRT_IMPL_EMPTY_BASES internal
    {};
    struct WINRT_IMPL_EMPTY_BASES override
    {
    };
}

#ifdef IDLGEN_CPP_STATIC_REFLECTION_PHASE
// Workaround winlibs gcc not having WinRT library. These are the only functions that need to have a stub given the list of
// function that winrt_base.h links to.
// Using __attribute__((weak)) to allow violating ODR by "allowing multiple definition and pick one" to simplify build
// Note that inline wouldn't work
// Ref: https://github.com/roxk/idlgen/issues/135
extern "C"
{
    __attribute__((weak)) HRESULT WINAPI RoGetActivationFactory(HSTRING classId, REFIID iid, void** factory)
    {
        return 0;
    }
    __attribute__((weak)) HRESULT WINAPI RoGetAgileReference(DWORD options, REFIID iid, IUnknown* object, void** reference)
    {
        return 0;
    }
    __attribute__((weak)) HRESULT WINAPI RoOriginateLanguageException(HRESULT error, HSTRING message, IUnknown* exception)
    {
        return 0;
    }
    __attribute__((weak)) HRESULT WINAPI RoCaptureErrorContext(HRESULT error)
    {
        return 0;
    }
    __attribute__((weak)) void WINAPI RoFailFastWithErrorContext(HRESULT error)
    {}
    __attribute__((weak)) HRESULT WINAPI RoTransformError(HRESULT oldError, HRESULT newError, HSTRING message)
    {
        return 0;
    }
}
#endif