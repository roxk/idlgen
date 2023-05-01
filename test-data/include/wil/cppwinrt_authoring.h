#pragma once

#include <utility>

// Copied directly from
// https://github.com/microsoft/wil/blob/1c6126b044fde6e7864b1273aa3d7247f1d2804a/include/wil/cppwinrt_authoring.h#L16
namespace wil
{
    namespace details
    {
        template<typename T>
        struct single_threaded_property_storage
        {
            T m_value{};
            single_threaded_property_storage() = default;
            single_threaded_property_storage(const T& value) : m_value(value) {}
            operator T& () { return m_value; }
            operator T const& () const { return m_value; }
            template<typename Q> auto operator=(Q&& q)
            {
                // Used to be wistd, changed to std to prevent having to copy everything from wil
                // Doesn't really affect test
                m_value = std::forward<Q>(q);
                return *this;
            }
        };
    }

    template <typename T>
    struct single_threaded_property : std::conditional_t<std::is_scalar_v<T> || std::is_final_v<T>, wil::details::single_threaded_property_storage<T>, T>
    {
        single_threaded_property() = default;
        template <typename... TArgs> single_threaded_property(TArgs&&... value) : base_type(std::forward<TArgs>(value)...) {}

        using base_type = std::conditional_t<std::is_scalar_v<T> || std::is_final_v<T>, wil::details::single_threaded_property_storage<T>, T>;

        const auto& operator()()
        {
            return *this;
        }

        template<typename Q> auto& operator()(Q&& q)
        {
            *this = std::forward<Q>(q);
            return *this;
        }

        template<typename Q> auto& operator=(Q&& q)
        {
            static_cast<base_type&>(*this) = std::forward<Q>(q);
            return *this;
        }
    };

    template <typename T>
    struct single_threaded_rw_property : single_threaded_property<T>
    {
        using base_type = single_threaded_property<T>;
        template<typename... TArgs> single_threaded_rw_property(TArgs&&... value) : base_type(std::forward<TArgs>(value)...) {}

        using base_type::operator();

        // needed in lieu of deducing-this
        template<typename Q> auto& operator()(Q&& q)
        {
            return *this = std::forward<Q>(q);
        }

        // needed in lieu of deducing-this
        template<typename Q> auto& operator=(Q&& q)
        {
            base_type::operator=(std::forward<Q>(q));
            return *this;
        }
    };
}