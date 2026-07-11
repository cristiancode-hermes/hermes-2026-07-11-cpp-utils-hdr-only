#pragma once

#include <type_traits>
#include <concepts>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

namespace cpp_utils {

// ── Concepts ──

// Numeric types (excluding bool)
template <typename T>
concept Numeric = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

// String-like types (string, string_view, C-strings)
template <typename T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

// Callable with no args
template <typename T>
concept Invocable = std::is_invocable_v<T>;

// Iterable container (has begin/end)
template <typename T>
concept Iterable = std::ranges::range<T>;

// Container of T
template <typename C, typename T>
concept ContainerOf = std::ranges::range<C> && std::is_same_v<std::ranges::range_value_t<C>, T>;

// Nullable pointer-like
template <typename T>
concept Nullable = requires(T t) {
  { t == nullptr } -> std::convertible_to<bool>;
  { t != nullptr } -> std::convertible_to<bool>;
};

// Has value_type (e.g., containers, optional, expected)
template <typename T>
concept HasValueType = requires { typename T::value_type; };

// Has error_type (e.g., expected)
template <typename T>
concept HasErrorType = requires { typename T::error_type; };

// ── Type traits ──

// Remove cvref + decay shortcut
template <typename T>
using decay_t = std::decay_t<T>;

// Check if all types in a pack are the same
template <typename T, typename... Rest>
inline constexpr bool all_same_v = (std::is_same_v<T, Rest> && ...);

// Check if any type in a pack matches T
template <typename T, typename... Rest>
inline constexpr bool any_same_v = (std::is_same_v<T, Rest> || ...);

// Type list contains
template <typename T, typename... Rest>
inline constexpr bool contains_type_v = (std::is_same_v<T, Rest> || ...);

// First type in pack
template <typename... Ts>
struct first_type;

template <typename T, typename... Rest>
struct first_type<T, Rest...> { using type = T; };

template <typename... Ts>
using first_type_t = typename first_type<Ts...>::type;

// Last type in pack
template <typename... Ts>
struct last_type;

template <typename T>
struct last_type<T> { using type = T; };

template <typename T, typename... Rest>
struct last_type<T, Rest...> { using type = typename last_type<Rest...>::type; };

template <typename... Ts>
using last_type_t = typename last_type<Ts...>::type;

// ── Detection helpers (like C++17 std::experimental::is_detected) ──
template <typename Default, typename AlwaysVoid, template <typename...> typename Op, typename... Args>
struct detector {
  using value_t = std::false_type;
  using type = Default;
};

template <typename Default, template <typename...> typename Op, typename... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
  using value_t = std::true_type;
  using type = Op<Args...>;
};

template <template <typename...> typename Op, typename... Args>
using is_detected = typename detector<void, void, Op, Args...>::value_t;

template <template <typename...> typename Op, typename... Args>
inline constexpr bool is_detected_v = is_detected<Op, Args...>::value;

// ── is_specialization: checks if T is a specialization of Template ──
template <typename T, template <typename...> typename Template>
inline constexpr bool is_specialization_v = false;

template <template <typename...> typename Template, typename... Args>
inline constexpr bool is_specialization_v<Template<Args...>, Template> = true;

// ── tuple_size_v for any type ──
template <typename T>
inline constexpr std::size_t tuple_size_v = std::tuple_size_v<std::remove_cvref_t<T>>;

// ── always_false (for static_assert in if constexpr) ──
template <typename... T>
inline constexpr bool always_false_v = false;

} // namespace cpp_utils
