#include "author_types.h"
#include "formatter.h"
#include "vector_string.h"
#include "winrt/author/base.h"
#include <cstring>
#include <iostream>
#include <meta>
#include <string>
#include <string_view>

using namespace std::string_literals;
using namespace std::string_view_literals;

enum class WinRtEntityType
{
    RuntimeClass,
    Interface,
    Delegate,
    Struct,
    Enum,
    Attribute
};

struct WinRtEntity
{
    WinRtEntityType type;
    std::meta::info info;
    consteval WinRtEntity(WinRtEntityType type, std::meta::info info) : type(type), info(info)
    {
    }
    consteval WinRtEntity(const WinRtEntity& that) : type(that.type), info(that.info)
    {
    }
};

consteval bool isAuthorNamespace(std::meta::info ns)
{
    // TODO: Handle upstream ns
    return (std::meta::has_identifier(ns) && std::meta::identifier_of(ns) == "author");
}

consteval vector_string fqn(std::meta::info info, bool isParameter = false);

template <typename Func = std::nullptr_t>
consteval vector_string fqn(std::meta::info info, bool isParameter = false, Func&& identitiferMapper = Func());

consteval void printParameters(std::vector<std::meta::info> const& params, vector_string& result)
{
    auto paramCount = params.size();
    auto paramIndex = 0;
    for (auto param : params)
    {
        auto type = std::meta::is_type(param) ? param : std::meta::type_of(param);
        auto paramName = std::meta::has_identifier(param) ? std::meta::identifier_of(param) : "";
        result += fqn(type, true);
        if (paramName.size() > 0)
        {
            result += " ";
            result += paramName;
        }
        if (paramIndex + 1 < paramCount)
        {
            result += ", ";
        }
        ++paramIndex;
    }
}

consteval bool is_winrt_category(std::meta::info type, std::meta::info category)
{
    if (!std::meta::is_type(type))
    {
        return false;
    }
    auto instantiatedCategoryT = std::meta::substitute(
        ^^winrt::impl::category_t,
        std::vector{
            type
        }
    );
    auto categoryValue = std::meta::dealias(instantiatedCategoryT);
    auto isCategoryTemplate = std::meta::has_template_arguments(categoryValue);
    return isCategoryTemplate ? std::meta::template_of(categoryValue) == category : categoryValue == category;
}

template <typename Func = decltype([](auto const& x) { return x; })>
consteval void printSelfName(
    std::meta::info candidate, vector_string& result, bool isParameter = false, Func&& identifierMapper = Func()
)
{
    if (std::meta::has_template_arguments(candidate))
    {
        auto templateType = std::meta::template_of(candidate);
        auto firstParam = std::meta::template_arguments_of(candidate)[0];
        if (templateType == ^^winrt::array_view)
        {
            if (isParameter && !std::meta::is_const_type(firstParam))
            {
                result += "ref ";
            }
            // Intentionally not applying mapper to template arguments. The mapper is only for the type itself
            result += fqn(firstParam, isParameter);
            result += "[]";
        }
        else if (templateType == ^^winrt::com_array)
        {
            if (isParameter)
            {
                result += "out ";
            }
            // Intentionally not applying mapper to template arguments. The mapper is only for the type itself
            result += fqn(firstParam, isParameter);
            result += "[]";
        }
        else
        {
            result += identifierMapper(
                std::meta::has_identifier(templateType) ? std::meta::identifier_of(templateType)
                                                        : std::meta::display_string_of(candidate)
            );
            result += "<";
            printParameters(std::meta::template_arguments_of(candidate), result);
            result += ">";
        }
    }
    else
    {
        result += identifierMapper(
            std::meta::has_identifier(candidate) ? std::meta::identifier_of(candidate)
                                                 : std::meta::display_string_of(candidate)
        );
    }
}

template <typename Func = decltype([](auto const& x) { return x; })>
consteval void printParentThenSelfName(
    std::meta::info candidate,
    vector_string& result,
    bool isParameter = false,
    std::string_view separator = ".",
    Func&& identifierMapper = Func()
)
{
    auto isAuthorNamespaceValue = isAuthorNamespace(candidate);
    if (std::meta::has_parent(candidate))
    {
        auto parent = std::meta::parent_of(candidate);
        if (isAuthorNamespace(parent))
        {
            // skip author namespace coz it should NOT be reflected in winmd
            if (std::meta::has_parent(parent))
            {
                parent = std::meta::parent_of(parent);
            }
            else
            {
                // something isn't right, should at least have a parent of global namespace, but
                // we can ignore this case for now
            }
        }
        if (parent != ^^::&&parent != ^^winrt)
        {
            printParentThenSelfName(parent, result, isParameter, separator, identifierMapper);
            if (!isAuthorNamespaceValue)
            {
                result += separator;
            }
        }
    }
    if (isAuthorNamespaceValue)
    {
        return;
    }
    printSelfName(candidate, result, isParameter, identifierMapper);
}

struct Int16
{
};
struct Int32
{
};
struct Int64
{
};
struct UInt8
{
};
struct UInt16
{
};
struct UInt32
{
};
struct UInt64
{
};
struct Char
{
};
struct String
{
};
struct Single
{
};
struct Double
{
};
struct Boolean
{
};
struct Guid
{
};
struct Object
{
};
namespace Windows::Foundation
{
struct DateTime
{
};
struct TimeSpan
{
};
} // namespace Windows::Foundation

static_assert(sizeof(wchar_t) == 2, "wchar_t is not 16 bit");
static_assert(sizeof(int8_t) == 1, "char is not 8 bit");
static_assert(sizeof(uint8_t) == 1, "char is not 8 bit");
static_assert(sizeof(short) == 2, "short is not 16 bit");
static_assert(sizeof(int) == 4, "int is not 32 bit");
static_assert(sizeof(long long) == 8, "long long is not 64 bit");

consteval std::meta::info toWinRtType(std::meta::info type)
{
    if (type == ^^short)
    {
        return ^^Int16;
    }
    else if (type == ^^int)
    {
        return ^^Int32;
    }
    else if (type == ^^long long)
    {
        return ^^Int64;
    }
    else if (type == ^^unsigned char)
    {
        return ^^UInt8;
    }
    else if (type == ^^unsigned short)
    {
        return ^^UInt16;
    }
    else if (type == ^^unsigned int)
    {
        return ^^UInt32;
    }
    else if (type == ^^unsigned long long)
    {
        return ^^UInt64;
    }
    else if (type == ^^wchar_t)
    {
        return ^^Char;
    }
    else if (type == ^^winrt::hstring)
    {
        return ^^String;
    }
    else if (type == ^^float)
    {
        return ^^Single;
    }
    else if (type == ^^double)
    {
        return ^^Double;
    }
    else if (type == ^^bool)
    {
        return ^^Boolean;
    }
    else if (type == ^^winrt::guid)
    {
        return ^^Guid;
    }
    else if (type == std::meta::dealias(^^winrt::Windows::Foundation::DateTime))
    {
        return ^^Windows::Foundation::DateTime;
    }
    else if (type == std::meta::dealias(^^winrt::Windows::Foundation::TimeSpan))
    {
        return ^^Windows::Foundation::TimeSpan;
    }
    else if (type == ^^winrt::Windows::Foundation::IInspectable)
    {
        return ^^Object;
    }
    return type;
}

constexpr auto expectedNameCount = 1024;

consteval vector_string fqn(std::meta::info info, bool isParameter)
{
    return fqn(info, isParameter, [](auto const& x) { return x; });
}

template <typename Func> consteval vector_string fqn(std::meta::info info, bool isParameter, Func&& identifierMapper)
{
    vector_string result;
    result.reserve(expectedNameCount);
    if (!std::meta::is_type(info))
    {
        throw std::runtime_error(std::meta::display_string_of(info) + " is not a type"s);
    }
    if (isParameter && is_winrt_category(std::meta::remove_cvref(info), ^^winrt::impl::struct_category) &&
        std::meta::is_reference_type(info) && std::meta::is_const_type(std::meta::remove_reference(info)))
    {
        result += "ref const ";
    }
    if (isParameter && std::meta::is_reference_type(info) &&
        !std::meta::is_const_type(std::meta::remove_reference(info)))
    {
        result += "out ";
    }
    auto winrtType = toWinRtType(std::meta::remove_cvref(info));
    if (std::meta::has_parent(winrtType) &&
        std::meta::parent_of(winrtType) == ^^winrt::Windows::Foundation::Collections)
    {
        printSelfName(winrtType, result, isParameter, identifierMapper);
    }
    else
    {
        printParentThenSelfName(winrtType, result, isParameter, ".", identifierMapper);
    }
    return result;
}

consteval void printFunctionParameters(std::meta::info member, vector_string& result)
{
    result += "(";
    printParameters(std::meta::parameters_of(member), result);
    result += ")";
}

consteval bool findBase(std::meta::info type, std::meta::info target, bool canTypeBeTemplate = false)
{
    auto ctx = std::meta::access_context::unchecked();
    if (!(canTypeBeTemplate && std::meta::is_template(type)))
    {
        if (!std::meta::is_complete_type(type))
        {
            return false;
        }
        if (!std::meta::is_class_type(type))
        {
            return false;
        }
    }
    auto bases = std::meta::bases_of(type, ctx);
    for (auto base : bases)
    {
        auto baseType = std::meta::type_of(base);
        if (std::meta::is_template(target))
        {
            if (std::meta::has_template_arguments(baseType) && std::meta::template_of(baseType) == target)
            {
                return true;
            }
        }
        else if (baseType == target)
        {
            return true;
        }
        else if (findBase(baseType, target))
        {
            return true;
        }
    }
    return false;
}

consteval bool isUnsealed(std::meta::info type)
{
    return findBase(type, ^^winrt::author::unsealed);
}

consteval bool isStaticClass(std::meta::info type)
{
    return findBase(type, ^^winrt::author::static_class);
}

consteval bool isPartialClass(std::meta::info type)
{
    return findBase(type, ^^winrt::author::partial);
}

consteval bool isRuntimeClass(std::meta::info type)
{
    return findBase(type, ^^winrt::author::runtimeclass);
}

consteval bool isDelegate(std::meta::info type)
{
    return findBase(type, ^^winrt::author::delegate);
}

consteval bool isStruct(std::meta::info type)
{
    return findBase(type, ^^winrt::author::winrt_struct);
}

consteval bool isInterface(std::meta::info type)
{
    return findBase(type, ^^winrt::author::winrt_interface);
}

consteval bool isEnum(std::meta::info type)
{
    if (!std::meta::is_scoped_enum_type(type))
    {
        return false;
    }
    auto underlying = std::meta::underlying_type(type);
    return underlying == ^^int || underlying == ^^unsigned int;
}

consteval bool isAttribute(std::meta::info type)
{
    return findBase(type, ^^winrt::author::attribute, true);
}

consteval bool isIgnoredEntity(std::meta::info info)
{
    return findBase(info, ^^winrt::author::ignore);
}

consteval bool isConstructor(std::meta::info member)
{
    if (!std::meta::is_constructor(member))
    {
        return false;
    }
    if (std::meta::is_copy_constructor(member) || std::meta::is_move_constructor(member))
    {
        return false;
    }
    return true;
}

consteval void handleAsConstructor(std::meta::info member, vector_string& result)
{
    auto parent = std::meta::parent_of(member);
    result += std::meta::identifier_of(parent);
    printFunctionParameters(member, result);
    result += ";\n";
}

consteval bool isFunction(std::meta::info member)
{
    if (!std::meta::is_function(member))
    {
        return false;
    }
    // TODO: If the method is overriding, return false
    auto returnType = std::meta::remove_cvref(std::meta::return_type_of(member));
    if (returnType == ^^winrt::event_token)
    {
        return false;
    }
    return true;
}

consteval void handleAsFunction(std::meta::info member, vector_string& result)
{
    auto memberType = std::meta::return_type_of(member);
    if (std::meta::is_static_member(member))
    {
        result += "static ";
    }
    auto isUnsealedValue = isUnsealed(std::meta::parent_of(member));
    if (isUnsealedValue && std::meta::is_virtual(member))
    {
        result += "overridable ";
    }
    if (isUnsealedValue && std::meta::is_protected(member))
    {
        result += "protected ";
    }
    result += fqn(memberType);
    result += " ";
    result += std::meta::identifier_of(member);
    printFunctionParameters(member, result);
    result += ";\n";
}

// template <int capacity>
// consteval void handleAsGetterByTemplateHelper(std::meta::info paramType,
//     std::meta::info member,
//     vector_string& result) {
//     result += std::meta::has_identifier(paramType)
//         ? std::meta::identifier_of(paramType)
//         : std::meta::display_string_of(paramType);
//     result += " ";
//     result += std::meta::identifier_of(member);
//     result += "{get;};\n";
// }

// template <int capacity>
// consteval void handleAsPropertyByTemplateHelper(std::meta::info paramType,
//     std::meta::info member,
//     vector_string& result) {
//     result += std::meta::has_identifier(paramType)
//         ? std::meta::identifier_of(paramType)
//         : std::meta::display_string_of(paramType);
//     result += " ";
//     result += std::meta::identifier_of(member);
//     result += "{get;set;};\n";
// }

// template <int capacity>
// consteval void handleAsEventByTemplateHelper(std::meta::info handlerType,
//     std::meta::info member,
//     vector_string& result) {
//     result += fqn<capacity>(handlerType);
//     if (std::meta::has_template_arguments(handlerType)) {
//         auto params = std::meta::template_arguments_of(handlerType);
//         if (params.size() > 0) {
//             result += "<";
//         }
//         auto paramCount = params.size();
//         auto paramIndex = 0;
//         for (auto param : params) {
//             result += fqn<capacity>(param);
//             if (paramIndex + 1 < paramCount) {
//                 result += ",";
//             }
//             ++paramIndex;
//         }
//         if (params.size() > 0) {
//             result += ">";
//         }
//     }
//     result += " ";
//     result += std::meta::identifier_of(member);
//     result += ";\n";
// }

consteval bool isGetter(std::meta::info member)
{
    if (!std::meta::is_function(member))
    {
        return false;
    }
    auto params = std::meta::parameters_of(member);
    auto paramCount = params.size();
    if (paramCount != 1)
    {
        return false;
    }
    auto firstParam = params[0];
    auto firstParamType = std::meta::remove_cvref(std::meta::type_of(firstParam));
    if (firstParamType != ^^winrt::author::getter)
    {
        return false;
    }
    return true;
}

consteval void handleAsGetter(std::meta::info member, vector_string& result)
{
    auto returnType = std::meta::return_type_of(member);
    result += fqn(returnType);
    result += " ";
    result += std::meta::identifier_of(member);
    result += " { get; };\n";
}

consteval bool isSetter(std::meta::info member)
{
    if (!std::meta::is_function(member))
    {
        return false;
    }
    auto params = std::meta::parameters_of(member);
    auto paramCount = params.size();
    if (paramCount != 1)
    {
        return false;
    }
    auto firstParam = params[0];
    auto firstParamType = std::meta::type_of(firstParam);
    auto isGetter = firstParamType == ^^winrt::author::getter;
    auto returnType = std::meta::return_type_of(member);
    if (returnType != ^^winrt::author::setter)
    {
        return false;
    }
    if (isGetter)
    {
        // TODO: error?
        return false;
    }
    return true;
}

consteval void handleAsProperty(std::meta::info getter, vector_string& result)
{
    if (std::meta::is_static_member(getter))
    {
        result += "static ";
    }
    auto returnType = std::meta::return_type_of(getter);
    result += fqn(returnType);
    result += " ";
    result += std::meta::identifier_of(getter);
    result += ";\n";
}

consteval bool isEventAdder(std::meta::info member)
{
    if (!std::meta::is_function(member))
    {
        return false;
    }
    auto returnType = std::meta::return_type_of(member);
    if (returnType != ^^winrt::event_token)
    {
        return false;
    }
    auto params = std::meta::parameters_of(member);
    auto paramCount = params.size();
    if (paramCount != 1)
    {
        return false;
    }
    auto firstParam = params[0];
    auto firstParamType = std::meta::remove_cvref(std::meta::type_of(firstParam));
    if (std::meta::has_template_arguments(firstParamType))
    {
        // It's found that all built in generic delegate extends IUnknown
        return findBase(firstParamType, ^^winrt::Windows::Foundation::IUnknown);
    }
    else
    {
        auto isParamDelegate = is_winrt_category(firstParamType, ^^winrt::impl::delegate_category);
        if (!isParamDelegate)
        {
            // TODO: Throw error?
            return false;
        }
    }
    return true;
}

consteval bool isEventRemover(std::meta::info member)
{
    if (!std::meta::is_function(member))
    {
        return false;
    }
    auto params = std::meta::parameters_of(member);
    auto paramCount = params.size();
    if (paramCount != 1)
    {
        return false;
    }
    auto firstParam = params[0];
    auto firstParamType = std::meta::remove_cvref(std::meta::type_of(firstParam));
    if (firstParamType == ^^winrt::event_token)
    {
        return true;
    }
    return false;
}

consteval void handleAsEvent(std::meta::info eventAdder, vector_string& result)
{
    auto params = std::meta::parameters_of(eventAdder);
    auto paramCount = params.size();
    auto firstParam = params[0];
    auto firstParamType = std::meta::type_of(firstParam);
    if (std::meta::is_static_member(eventAdder))
    {
        result += "static ";
    }
    result += "event ";
    result += fqn(firstParamType);
    result += " ";
    result += std::meta::identifier_of(eventAdder);
    result += ";\n";
}

consteval bool HandleAsNsdm(std::meta::info member, vector_string& result)
{
    if (!std::meta::is_nonstatic_data_member(member))
    {
        return false;
    }
    auto memberType = std::meta::type_of(member);
    if (!has_template_arguments(memberType))
    {
        return false;
    }
    auto templateType = std::meta::template_of(memberType);
    auto params = std::meta::template_arguments_of(memberType);
    // if (templateType == ^^wil::single_threaded_property) {
    //     auto param = params[0];
    //     handleAsGetterByTemplateHelper(param, member, result);
    // }
    // else if (templateType == ^^wil::single_threaded_rw_property) {
    //     auto param = params[0];
    //     handleAsPropertyByTemplateHelper(param, member, result);
    // }
    // else if (templateType == ^^winrt::event) {
    //     auto param = params[0];
    //     handleAsEventByTemplateHelper(param, member, result);
    // }
    return true;
}

consteval bool isBaseTypeWinRtBase(std::meta::info type)
{
    if (!std::meta::is_type(type))
    {
        return false;
    }
    if (std::meta::has_template_arguments(type))
    {
        return std::meta::template_of(type) == ^^winrt::author::runtimeclass;
    }
    return isInterface(type) || isAttribute(type);
}

consteval void trimIgnoredBaseType(std::vector<std::meta::info>& bases)
{
    bases.erase(
        std::remove_if(
            bases.begin(),
            bases.end(),
            [](std::meta::info base)
            {
                auto baseType = std::meta::type_of(base);
                return !isBaseTypeWinRtBase(baseType);
            }
        ),
        bases.end()
    );
}

consteval std::optional<WinRtEntity> handleAsInterface(std::meta::info info)
{
    if (!std::meta::is_type(info))
    {
        return std::nullopt;
    }
    if (!isInterface(info))
    {
        return std::nullopt;
    }
    if (isIgnoredEntity(info))
    {
        return std::nullopt;
    }
    return WinRtEntity(WinRtEntityType::Interface, info);
}

consteval std::optional<WinRtEntity> handleAsRuntimeClass(std::meta::info info)
{
    if (!std::meta::is_type(info))
    {
        return std::nullopt;
    }
    if (!isRuntimeClass(info))
    {
        return std::nullopt;
    }
    if (isIgnoredEntity(info))
    {
        return std::nullopt;
    }
    return WinRtEntity(WinRtEntityType::RuntimeClass, info);
}

consteval std::optional<WinRtEntity> handleAsDelegate(std::meta::info info)
{
    if (!std::meta::is_type(info))
    {
        return std::nullopt;
    }
    if (!isDelegate(info))
    {
        return std::nullopt;
    }
    if (isIgnoredEntity(info))
    {
        return std::nullopt;
    }
    return WinRtEntity(WinRtEntityType::Delegate, info);
}

consteval std::optional<WinRtEntity> handleAsStruct(std::meta::info info)
{
    if (!std::meta::is_type(info))
    {
        return std::nullopt;
    }
    if (!isStruct(info))
    {
        return std::nullopt;
    }
    if (isIgnoredEntity(info))
    {
        return std::nullopt;
    }
    return WinRtEntity(WinRtEntityType::Struct, info);
}

consteval std::optional<WinRtEntity> handleAsEnum(std::meta::info info)
{
    if (!std::meta::is_type(info))
    {
        return std::nullopt;
    }
    if (!isEnum(info))
    {
        return std::nullopt;
    }
    if (isIgnoredEntity(info))
    {
        return std::nullopt;
    }
    return WinRtEntity(WinRtEntityType::Enum, info);
}

consteval std::optional<WinRtEntity> handleAsAttribute(std::meta::info info)
{
    if (!std::meta::is_type(info))
    {
        return std::nullopt;
    }
    if (!isAttribute(info))
    {
        return std::nullopt;
    }
    if (isIgnoredEntity(info))
    {
        return std::nullopt;
    }
    return WinRtEntity(WinRtEntityType::Attribute, info);
}

consteval bool handleAsNamespace(std::meta::info info)
{
    if (!std::meta::is_namespace(info))
    {
        return false;
    }
    return true;
}

template <typename T> consteval std::string toString(T size)
{
    constexpr auto bufferSize = 256;
    char buf[bufferSize];
    auto [ptr, ec] = std::to_chars(buf, buf + bufferSize, size);
    return std::string(buf, ptr - buf);
}

consteval void findWinRtEntities(std::meta::info ns, std::vector<WinRtEntity>& entities)
{
    auto ctx = std::meta::access_context::unchecked();
    const auto isAuthorNamespaceValue = isAuthorNamespace(ns);
    auto members = std::meta::members_of(ns, ctx);
    for (auto member : members)
    {
        if (handleAsNamespace(member))
        {
            findWinRtEntities(member, entities);
            continue;
        }
        if (!isAuthorNamespaceValue)
        {
            continue;
        }
        if (auto classResult = handleAsRuntimeClass(member); classResult.has_value())
        {
            entities.push_back(*classResult);
            continue;
        }
        else if (auto interfaceResult = handleAsInterface(member); interfaceResult.has_value())
        {
            entities.push_back(*interfaceResult);
            continue;
        }
        else if (auto delegateResult = handleAsDelegate(member); delegateResult.has_value())
        {
            entities.push_back(*delegateResult);
            continue;
        }
        else if (auto structResult = handleAsStruct(member); structResult.has_value())
        {
            entities.push_back(*structResult);
            continue;
        }
        else if (auto enumResult = handleAsEnum(member); enumResult.has_value())
        {
            entities.push_back(*enumResult);
            continue;
        }
        else if (auto attributeResult = handleAsAttribute(member); attributeResult.has_value())
        {
            entities.push_back(*attributeResult);
            continue;
        }
    }
}

consteval void printNamespace(std::meta::info info, vector_string& result, std::string_view separator = ".")
{
    result += "namespace ";
    printParentThenSelfName(std::meta::parent_of(info), result, false, separator);
    result += " {\n";
}

consteval void printNamespaceOnly(std::meta::info info, vector_string& result, std::string_view separator = ".")
{
    printParentThenSelfName(std::meta::parent_of(info), result, false, separator);
}

consteval bool isIgnored(std::meta::info member)
{
    if (!std::meta::is_function(member))
    {
        return false;
    }
    auto params = std::meta::parameters_of(member);
    for (auto param : params)
    {
        auto paramType = std::meta::type_of(param);
        if (paramType == ^^winrt::author::ignore)
        {
            return true;
        }
    }
    return false;
}

struct ClassMemberInfo
{
    std::string name;
    std::optional<std::meta::info> getter;
    std::optional<std::meta::info> setter;
    std::optional<std::meta::info> eventAdder;
    std::optional<std::meta::info> eventRemover;
    std::optional<std::meta::info> constructor;
    std::optional<std::meta::info> method;
    bool handled{};
    consteval ClassMemberInfo() = default;
    consteval ClassMemberInfo(std::string name) : name(std::move(name))
    {
    }
    consteval ClassMemberInfo(ClassMemberInfo&& that) = default;
    consteval ClassMemberInfo& operator=(ClassMemberInfo&& that) = default;
};

consteval std::string getMemberName(std::meta::info info)
{
    if (std::meta::is_function(info) || std::meta::is_constructor(info))
    {
        auto name = std::meta::is_constructor(info) ? std::meta::identifier_of(std::meta::parent_of(info))
                                                    : std::meta::identifier_of(info);
        return std::string(name) + "_" + toString(std::meta::parameters_of(info).size());
    }
    // TODO: Handle nsdm template helper
    return std::string(std::meta::identifier_of(info));
}

// TODO: Make it O(1)
template <typename Func>
consteval ClassMemberInfo& insertOr(std::vector<ClassMemberInfo>& infos, std::meta::info member, Func&& predicate)
{
    auto memberName = getMemberName(member);
    for (auto& info : infos)
    {
        if (info.name == memberName)
        {
            if constexpr (std::is_same_v<decltype(predicate(info, memberName)), void>)
            {
                predicate(info, memberName);
            }
            else
            {
                return predicate(info, memberName);
            }
        }
    }
    infos.emplace_back(std::move(memberName));
    return infos.back();
}

consteval ClassMemberInfo& insertOrGet(std::vector<ClassMemberInfo>& infos, std::meta::info member)
{
    return insertOr(
        infos, member, [](ClassMemberInfo& info, std::string_view) consteval -> ClassMemberInfo& { return info; }
    );
}

consteval ClassMemberInfo& insertOrThrow(std::vector<ClassMemberInfo>& infos, std::meta::info member)
{
    return insertOr(
        infos,
        member,
        [](ClassMemberInfo& info, std::string_view memberName) consteval
        { throw std::runtime_error("WinRT does not support overload with the same arity: "s + memberName); }
    );
}

consteval void printMemberInfos(std::meta::info type, std::vector<ClassMemberInfo>& infos, vector_string& result)
{
    auto ctx = std::meta::access_context::unchecked();
    auto members = std::meta::members_of(type, ctx);
    for (auto member : members)
    {
        const auto isContructorValue = isConstructor(member);
        if (!std::meta::has_identifier(member) && !isContructorValue)
        {
            continue;
        }
        auto memberName = getMemberName(member);
        std::optional<std::reference_wrapper<ClassMemberInfo>> infoOpt;
        for (auto& info : infos)
        {
            if (info.name == memberName)
            {
                infoOpt = info;
                break;
            }
        }
        if (!infoOpt.has_value())
        {
            continue;
        }
        auto& memberInfo = infoOpt.value().get();
        if (memberInfo.handled)
        {
            continue;
        }
        if (memberInfo.constructor.has_value())
        {
            handleAsConstructor(memberInfo.constructor.value(), result);
        }
        else if (memberInfo.getter.has_value() && memberInfo.setter.has_value())
        {
            handleAsProperty(memberInfo.getter.value(), result);
        }
        else if (memberInfo.getter.has_value())
        {
            handleAsGetter(memberInfo.getter.value(), result);
        }
        else if (memberInfo.eventAdder.has_value() && memberInfo.eventRemover.has_value())
        {
            handleAsEvent(memberInfo.eventAdder.value(), result);
        }
        else if (memberInfo.method.has_value())
        {
            handleAsFunction(memberInfo.method.value(), result);
        }
        else
        {
            // TODO: error?
            continue;
        }
        memberInfo.handled = true;
    }
}

template <std::meta::info type> consteval void printApplyingAttribute(vector_string& result)
{
    constexpr static auto args = std::define_static_array(std::meta::template_arguments_of(type));
    result += "[";
    auto argCount = args.size();
    if (argCount == 0)
    {
        throw std::runtime_error(std::meta::display_string_of(type) + " attribute must have at least one parameter"s);
    }
    auto attrParam = args[0];
    if (!std::meta::is_type(attrParam))
    {
        throw std::runtime_error(
            "the first parameter of an attribute must be a type, got "s + std::meta::display_string_of(attrParam)
        );
    }
    result +=
        fqn(attrParam,
            false,
            [](std::string_view name) -> std::string_view
            {
                constexpr auto attributeSuffix = "Attribute"sv;
                auto attributeIndex = name.find(attributeSuffix);
                if (attributeIndex == std::string_view::npos)
                {
                    return name;
                }
                auto isAttributeSuffix = (attributeIndex + attributeSuffix.size()) == name.size();
                return isAttributeSuffix ? name.substr(0, attributeIndex) : name;
            });
    if (argCount > 1)
    {
        result += "(";
        constexpr static auto attrParams = std::span(args).subspan(1);
        auto paramIndex = 0;
        auto paramCount = attrParams.size();
        template for (constexpr auto param : attrParams)
        {
            if (std::meta::is_type(param))
            {
                throw std::runtime_error(
                    "attribute parameters must be NTTP, got "s + std::meta::display_string_of(param)
                );
            }
            constexpr auto paramType = std::meta::type_of(param);
            if constexpr (std::meta::is_integral_type(paramType))
            {
                result += toString([:param:]);
            }
            else if constexpr (std::meta::has_template_arguments(paramType) &&
                               std::meta::template_of(paramType) == ^^winrt::author::attr_string)
            {
                result += "\"";
                result += [:param:].data;
                result += "\"";
            }
            else if constexpr (std::meta::has_template_arguments(paramType) &&
                               std::meta::template_of(paramType) == ^^winrt::author::attr_type)
            {
                result += [:param:].data;
            }
            else if constexpr (std::meta::remove_const(paramType) == ^^winrt::guid)
            {
                auto g = [:param:];
                auto data = guid_to_string(g);
                result += std::string_view(data.data());
            }
            else
            {
                throw std::runtime_error(
                    "unsupported attribute parameter type: "s + std::meta::display_string_of(paramType)
                );
            }
            if (paramIndex + 1 < paramCount)
            {
                result += ", ";
            }
            ++paramIndex;
        }
        result += ")";
    }
    result += "]\n";
}

consteval std::vector<std::meta::info> getAttributeBases(std::meta::info info)
{
    auto bases = std::meta::bases_of(info, std::meta::access_context::unchecked());
    bases.erase(
        std::remove_if(
            bases.begin(),
            bases.end(),
            [](std::meta::info base)
            {
                auto baseType = std::meta::type_of(base);
                return !(
                    std::meta::has_template_arguments(baseType) &&
                    std::meta::template_of(baseType) == ^^winrt::author::apply_attr
                );
            }
        ),
        bases.end()
    );
    return bases;
}

template <std::meta::info type> consteval void printBaseAttributes(vector_string& result)
{
    constexpr static auto attributeBases = std::define_static_array(getAttributeBases(type));
    template for (constexpr auto base : attributeBases)
    {
        constexpr auto baseType = std::meta::type_of(base);
        if constexpr (std::meta::is_complete_type(baseType) && std::meta::is_class_type(baseType))
        {
            printApplyingAttribute<baseType>(result);
        }
    }
}

consteval void tryInclude(vector_string& result, std::string_view what)
{
    result += "#if __has_include(\"";
    result += what;
    result += "\")\n";
    result += "#include \"";
    result += what;
    result += "\"\n";
    result += "#endif\n";
}

template <std::meta::info info> consteval void printRuntimeClass(vector_string& idl, vector_string& implementation, vector_string& implementationCpp)
{
    constexpr auto type = info;
    printNamespace(type, idl);
    printBaseAttributes<type>(idl);
    idl += "[default_interface]\n";
    constexpr auto isStaticClassValue = isStaticClass(type);
    constexpr auto isUnsealedValue = isUnsealed(type);
    if constexpr (isUnsealedValue)
    {
        idl += "unsealed ";
    }
    else if constexpr (isStaticClassValue)
    {
        idl += "static ";
    }
    if (isPartialClass(type))
    {
        idl += "partial ";
    }
    idl += "runtimeclass ";
    auto typeName = std::meta::identifier_of(type);
    idl += typeName;
    constexpr auto ctx = std::meta::access_context::unchecked();
    auto bases = std::meta::bases_of(type, ctx);
    trimIgnoredBaseType(bases);
    bool hasBases = false;
    for (auto base : bases)
    {
        auto baseType = std::meta::type_of(base);
        if (std::meta::has_template_arguments(baseType) &&
            std::meta::template_of(baseType) == ^^winrt::author::runtimeclass)
        {
            if (std::meta::template_arguments_of(baseType).size() > 0)
            {
                hasBases = true;
            }
            break;
        }
        else
        {
            // has to be interface
            hasBases = true;
            break;
        }
    }
    if (hasBases)
    {
        idl += " : ";
    }
    auto basesCount = bases.size();
    auto baseIndex = 0;
    std::optional<std::meta::info> runtimeClassBase;
    for (auto base : bases)
    {
        auto baseType = std::meta::type_of(base);
        if (std::meta::has_template_arguments(baseType) &&
            std::meta::template_of(baseType) == ^^winrt::author::runtimeclass)
        {
            basesCount -= 1; // remove the runtimeclass base itself, we will add its template args as bases
            auto args = std::meta::template_arguments_of(baseType);
            basesCount += args.size();
            for (auto arg : args)
            {
                if (is_winrt_category(arg, ^^winrt::impl::class_category))
                {
                    runtimeClassBase = arg;
                }
                idl += fqn(arg);
                if (baseIndex + 1 < basesCount)
                {
                    idl += ", ";
                }
                ++baseIndex;
            }
            continue;
        }
        idl += fqn(baseType);
        if (baseIndex + 1 < basesCount)
        {
            idl += ", ";
        }
        ++baseIndex;
    }
    idl += " {\n";
    auto members = std::meta::members_of(type, ctx);
    std::vector<ClassMemberInfo> infos;
    bool hasCtor = false;
    bool hasProtected = false;
    for (auto member : members)
    {
        if (isIgnored(member))
        {
            continue;
        }
        else if (isConstructor(member) && !isStaticClassValue)
        {
            hasCtor = true;
            insertOrGet(infos, member).constructor = member;
        }
        else if (std::meta::is_special_member_function(member))
        {
            continue;
        }
        else if (isGetter(member))
        {
            insertOrGet(infos, member).getter = member;
        }
        else if (isSetter(member))
        {
            insertOrGet(infos, member).setter = member;
        }
        else if (isEventAdder(member))
        {
            insertOrGet(infos, member).eventAdder = member;
        }
        else if (isEventRemover(member))
        {
            insertOrGet(infos, member).eventRemover = member;
        }
        else if (isFunction(member))
        {
            if (std::meta::is_protected(member) && isUnsealedValue)
            {
                hasProtected = true;
            }
            insertOrThrow(infos, member).method = member;
        }
    }
    printMemberInfos(type, infos, idl);
    idl += "}\n";
    idl += "}\n";
    // heap implements
    tryInclude(implementation, typeName + ".g.h"s);
    implementation += "namespace winrt::";
    printNamespaceOnly(type, implementation);
    implementation += "::implementation {\n";
    implementation += "struct ";
    implementation += typeName;
    implementation += "Heap : author::";
    implementation += typeName;
    implementation += " {\n";
    implementation += "using author::";
    implementation += typeName;
    implementation += "::";
    implementation += typeName;
    implementation += ";\n";
    implementation += "void use_make_function_to_create_this_object() {}\n";
    implementation += "};\n";
    // ClassT
    implementation += "struct ";
    implementation += typeName;
    implementation += " : ";
    implementation += typeName;
    implementation += "T<";
    implementation += typeName;
    implementation += ">, ";
    implementation += typeName;
    implementation += "Heap {\n";
    implementation += "using ";
    implementation += typeName;
    implementation += "Heap::";
    implementation += typeName;
    implementation += "Heap;\n";
    auto printDeclareBaseFromThis = [&]()
    {
        implementation += std::meta::display_string_of(*runtimeClassBase);
        implementation += "& base = *static_cast<";
        implementation += typeName;
        implementation += "Heap*>(this);\n";
    };
    if (runtimeClassBase.has_value())
    {
        // TODO: Use AddRef or detect ctor?
        implementation += typeName;
        implementation += "() {\n    ";
        printDeclareBaseFromThis();
        implementation += "    base = m_inner.try_as<";
        implementation += std::meta::display_string_of(*runtimeClassBase);
        implementation += ">();\n";
        implementation += "}\n";
        implementation += R""(uint32_t __stdcall Release() {
    auto const count = this->implements::Release();
    if (count == 1) {
        )"";
        printDeclareBaseFromThis();
        implementation += R""(        base = nullptr;
    }
    return count;
}
void* operator new(std::size_t) {
    void* mem = std::malloc(sizeof()"";
        implementation += typeName;
        implementation += R""());
    return mem;
})"";
        implementation += "\n";
    }
    implementation += "friend struct winrt::impl::produce<";
    implementation += typeName;
    implementation += ", winrt::";
    printNamespaceOnly(type, implementation);
    implementation += "::I";
    implementation += typeName;
    implementation += ">;\n";
    if (hasProtected)
    {

        implementation += "friend struct winrt::impl::produce<";
        implementation += typeName;
        implementation += ", winrt::";
        printNamespaceOnly(type, implementation);
        implementation += "::I";
        implementation += typeName;
        implementation += "Protected>;\n";
    }
    implementation += "};\n";
    implementation += "}\n";
    if (hasCtor)
    {
        implementation += "namespace winrt::";
        printNamespaceOnly(type, implementation);
        implementation += "::factory_implementation {\n";
        implementation += "struct ";
        implementation += typeName;
        implementation += " : ";
        implementation += typeName;
        implementation += "T<";
        implementation += typeName;
        implementation += ", implementation::";
        implementation += typeName;
        implementation += "> {};\n";
        implementation += "}\n";
    }
    tryInclude(implementationCpp, typeName + ".g.cpp"s);
    tryInclude(implementationCpp, typeName + ".xaml.g.hpp"s);
}

template <std::meta::info type> consteval void printInterface(vector_string& idl, vector_string& implementation)
{
    printNamespace(type, idl);
    printBaseAttributes<type>(idl);
    idl += "interface ";
    idl += std::meta::identifier_of(type);
    auto ctx = std::meta::access_context::unchecked();
    auto bases = std::meta::bases_of(type, ctx);
    trimIgnoredBaseType(bases);
    auto basesCount = bases.size();
    if (basesCount > 0)
    {
        idl += " requires ";
    }
    auto baseCount = bases.size();
    auto baseIndex = 0;
    for (auto base : bases)
    {
        auto baseType = std::meta::type_of(base);
        idl += fqn(baseType);
        if (baseIndex + 1 < baseCount)
        {
            idl += ", ";
        }
        ++baseIndex;
    }
    idl += " {\n";
    auto members = std::meta::members_of(type, ctx);
    std::vector<ClassMemberInfo> infos;
    for (auto member : members)
    {
        if (std::meta::is_special_member_function(member))
        {
            continue;
        }
        else if (isIgnored(member))
        {
            continue;
        }
        else if (isGetter(member))
        {
            insertOrGet(infos, member).getter = member;
        }
        else if (isSetter(member))
        {
            insertOrGet(infos, member).setter = member;
        }
        else if (isEventAdder(member))
        {
            insertOrGet(infos, member).eventAdder = member;
        }
        else if (isEventRemover(member))
        {
            insertOrGet(infos, member).eventRemover = member;
        }
        else if (isFunction(member))
        {
            insertOrThrow(infos, member).method = member;
        }
    }
    printMemberInfos(type, infos, idl);
    idl += "}\n";
    idl += "}\n";
}

consteval void printDelegate(vector_string& result, std::meta::info info)
{
    auto type = info;
    printNamespace(type, result);
    result += "delegate void ";
    result += std::meta::identifier_of(type);
    auto ctx = std::meta::access_context::unchecked();
    auto members = std::meta::members_of(type, ctx);
    auto memberCount = members.size();
    auto constructorCount = 0;
    for (auto member : members)
    {
        if (!std::meta::is_constructor(member))
        {
            continue;
        }
        if (std::meta::parameters_of(member).size() != 2)
        {
            continue;
        }
        if (constructorCount > 0)
        {
            throw std::runtime_error("Delegate should only have one constructor with two parameters");
        }
        printFunctionParameters(member, result);
        result += ";\n";
        ++constructorCount;
    }
    result += "}\n";
}

consteval void printStruct(vector_string& result, std::meta::info info)
{
    auto type = info;
    printNamespace(type, result);
    result += "struct ";
    result += std::meta::identifier_of(type);
    result += " {\n";
    auto ctx = std::meta::access_context::unchecked();
    auto members = std::meta::members_of(type, ctx);
    for (auto member : members)
    {
        if (std::meta::is_special_member_function(member))
        {
            continue;
        }
        if (!std::meta::is_nonstatic_data_member(member))
        {
            throw std::runtime_error("Struct should only have non-static data members");
            continue;
        }
        auto memberType = std::meta::type_of(member);
        result += fqn(memberType);
        result += " ";
        result += std::meta::identifier_of(member);
        result += ";\n";
    }
    result += "};\n";
    result += "}\n";
}

template <std::meta::info info> consteval void printEnum(vector_string& result)
{
    constexpr auto type = info;
    printNamespace(type, result);
    auto isFlags = std::meta::underlying_type(type) == ^^unsigned int;
    if (isFlags)
    {
        result += "[flags]\n";
    }
    result += "enum ";
    result += std::meta::identifier_of(type);
    result += " {\n";
    constexpr static auto members = std::define_static_array(std::meta::enumerators_of(type));
    auto memberCount = members.size();
    auto memberIndex = 0;
    template for (constexpr auto member : members)
    {
        result += std::meta::identifier_of(member);
        result += " = ";
        auto enumValue = [:std::meta::constant_of(member):];
        result += toString(static_cast<std::underlying_type_t<decltype(enumValue)>>(enumValue));
        if (memberIndex + 1 < memberCount)
        {
            result += ",";
        }
        result += "\n";
        ++memberIndex;
    }
    result += "};\n";
    result += "}\n";
}

consteval void printAttribute(vector_string& result, std::meta::info info)
{
    auto type = info;
    printNamespace(type, result);
    auto ctx = std::meta::access_context::unchecked();
    auto bases = std::meta::bases_of(type, ctx);
    for (auto base : bases)
    {
        auto baseType = std::meta::type_of(base);
        if (!(std::meta::has_template_arguments(baseType) &&
              std::meta::template_of(baseType) == ^^winrt::author::attribute))
        {
            continue;
        }
        auto attrArgs = std::meta::template_arguments_of(baseType);
        for (auto attrArg : attrArgs)
        {
            if (attrArg == ^^winrt::author::allowmultiple)
            {
                result += "[allowmultiple]\n";
            }
            else if (std::meta::has_template_arguments(attrArg) &&
                     std::meta::template_of(attrArg) == ^^winrt::author::attributeusage)
            {
                auto args = std::meta::template_arguments_of(attrArg);
                if (args.size() != 0)
                {
                    result += "[attributeusage(";
                }
                auto argCount = args.size();
                auto argIndex = 0;
                for (auto arg : args)
                {
                    if (std::meta::is_base_of_type(^^winrt::author::attribute_target, arg))
                    {
                        result += std::meta::identifier_of(arg);
                    }
                    if (argIndex + 1 < argCount)
                    {
                        result += ", ";
                    }
                    ++argIndex;
                }
                if (args.size() != 0)
                {
                    result += ")]\n";
                }
            }
        }
        break;
    }
    result += "attribute ";
    result += std::meta::identifier_of(type);
    result += " {\n";
    auto members = std::meta::members_of(type, ctx);
    for (auto member : members)
    {
        if (!std::meta::is_constructor(member))
        {
            continue;
        }
        auto params = std::meta::parameters_of(member);
        for (auto param : params)
        {
            auto type = std::meta::type_of(param);
            result += fqn(type);
            result += " ";
            result += std::meta::identifier_of(param);
            result += ";\n";
        }
        break;
    }
    result += "}\n";
    result += "}\n";
}

constexpr auto expectedEntityCount = 4096;

consteval std::vector<WinRtEntity> getEntites()
{
    std::vector<WinRtEntity> entities;
    entities.reserve(expectedEntityCount);
    findWinRtEntities(^^winrt, entities);
    return entities;
}

constexpr auto expectedOutputSize = 8096;
constexpr auto expectedImplementationFileSize = expectedOutputSize;

struct GenResult
{
    vector_string idl;
    vector_string implementation;
    vector_string implementationCpp;
};

consteval GenResult gen_idl_impl()
{
    vector_string idl;
    vector_string implementation;
    vector_string implementationCpp;
    idl.reserve(expectedOutputSize);
    implementation.reserve(expectedImplementationFileSize);
    implementationCpp.reserve(expectedImplementationFileSize);
    constexpr static auto entities = std::define_static_array(getEntites());
    template for (constexpr auto entity : entities)
    {
        constexpr auto type = entity.type;
        constexpr auto info = entity.info;
        if constexpr (type == WinRtEntityType::RuntimeClass)
        {
            printRuntimeClass<info>(idl, implementation, implementationCpp);
        }
        else if constexpr (type == WinRtEntityType::Interface)
        {
            printInterface<info>(idl, implementation);
        }
        else if constexpr (type == WinRtEntityType::Delegate)
        {
            printDelegate(idl, info);
        }
        else if constexpr (type == WinRtEntityType::Struct)
        {
            printStruct(idl, info);
        }
        else if constexpr (type == WinRtEntityType::Enum)
        {
            printEnum<info>(idl);
        }
        else if constexpr (type == WinRtEntityType::Attribute)
        {
            printAttribute(idl, info);
        }
    }
    return { idl, implementation, implementationCpp };
}

consteval auto gen_idl()
{
    auto result = gen_idl_impl();
    return std::tuple(
        std::define_static_array(result.idl.data()), 
        std::define_static_array(result.implementation.data()),
        std::define_static_array(result.implementationCpp.data())
    );
}

constexpr auto generated = gen_idl();

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cerr << "expected one argument. usage: .exe -idl|-implementation" << std::endl;
        return -1;
    }
    if (argc > 2)
    {
        std::cerr << "unknown argument" << argv[2] << std::endl;
        return -1;
    }
    auto arg = std::string_view(argv[1]);
    auto& [idl, implementation, implementationCpp] = generated;
    if (arg == "-idl")
    {
        std::cout << std::string(idl.data(), idl.size()) << std::endl;
    }
    else if (arg == "-implementation-header")
    {
        std::cout << "#pragma once" << std::endl;
        std::cout << "#include \"pch.h\"" << std::endl;
        std::cout << "#include \"author_types.h\"" << std::endl;
        std::cout << std::string(implementation.data(), implementation.size()) << std::endl;
    }
    else if (arg == "-implementation-cpp")
    {
        std::cout << "#include \"pch.h\"" << std::endl;
        std::cout << "#include \"idlgen.impl.h\"" << std::endl;
        std::cout << std::string(implementationCpp.data(), implementationCpp.size()) << std::endl;
    }
    else
    {
        std::cerr << "unknown argument" << arg << std::endl;
        return -1;
    }
    return 0;
}
