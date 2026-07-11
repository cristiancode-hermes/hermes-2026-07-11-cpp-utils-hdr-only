#pragma once

#include <expected>
#include <string>
#include <format>
#include <functional>
#include <source_location>
#include <system_error>
#include <type_traits>

namespace cpp_utils {

// ── Extended utilities for std::expected ──

// A simple error type with code + message
struct Error {
  int         code    = 0;
  std::string message = {};

  // Build from errc
  explicit Error(std::errc ec) noexcept
    : code(static_cast<int>(ec)), message(std::generic_category().message(static_cast<int>(ec))) {}

  Error(int c, std::string msg) : code(c), message(std::move(msg)) {}

  // Implicit from string (code=1)
  Error(std::string msg) : code(1), message(std::move(msg)) {}
  Error(const char* msg) : code(1), message(msg) {}

  // Format with source location
  static Error at(std::string msg,
                  std::source_location loc = std::source_location::current()) {
    return Error{1, std::format("{}:{}: {}",
                                loc.file_name(), loc.line(), msg)};
  }
};

// Type alias
template <typename T>
using Result = std::expected<T, Error>;

// ── then: chain success paths ──
//   Result<int> r = parse_int("42").then([](int v) { return v * 2; });
template <typename T, typename Fn>
  requires std::is_invocable_v<Fn, T>
auto then(Result<T>&& result, Fn&& fn) -> Result<std::invoke_result_t<Fn, T>> {
  if (result) {
    if constexpr (std::is_same_v<std::invoke_result_t<Fn, T>, void>) {
      std::invoke(std::forward<Fn>(fn), *result);
      return {};
    } else {
      return std::invoke(std::forward<Fn>(fn), *result);
    }
  }
  return std::unexpected(result.error());
}

// ── or_else: handle errors ──
//   Result<int> r = parse_int("abc").or_else([](Error) { return Result<int>(0); });
template <typename T, typename Fn>
  requires std::is_invocable_v<Fn, Error>
auto or_else(Result<T>&& result, Fn&& fn) -> Result<T> {
  if (!result) {
    return std::invoke(std::forward<Fn>(fn), result.error());
  }
  return std::move(result);
}

// ── map_error: transform error type ──
template <typename T, typename Fn>
  requires std::is_invocable_v<Fn, Error>
auto map_error(Result<T>&& result, Fn&& fn) -> std::expected<T, std::invoke_result_t<Fn, Error>> {
  if (result) return *result;
  return std::unexpected(std::invoke(std::forward<Fn>(fn), result.error()));
}

// ── try_all: try a sequence of fallible operations, return first success ──
template <typename T, typename... Fns>
  requires (std::is_invocable_v<Fns> && ...)
Result<T> try_all(Fns&&... fns) {
  Result<T> last_error = std::unexpected(Error{0, "no alternatives"});
  auto try_one = [&](auto&& fn) {
    if (last_error) return;
    auto r = fn();
    if (r) { last_error = std::move(r); }
    else { last_error = std::unexpected(r.error()); }
  };
  (try_one(std::forward<Fns>(fns)), ...);
  return last_error;
}

// ── Collect all expected values (or first error) ──
template <typename... Ts>
Result<std::tuple<Ts...>> collect(Result<Ts>... results) {
  auto check = [](auto& r) -> bool { return r.has_value(); };
  if ((check(results) && ...)) {
    return std::tuple(std::move(*results)...);
  }
  // Return first error
  auto first_err = [](auto& r) -> std::optional<Error> {
    if (!r) return r.error();
    return std::nullopt;
  };
  std::optional<Error> err;
  ((err = first_err(results)).has_value() || ...);
  return std::unexpected(err.value_or(Error{0, "unknown error"}));
}

// ── Wrap a throwing callable into expected ──
template <typename Fn>
  requires std::is_invocable_v<Fn>
auto wrap_except(Fn&& fn) -> Result<std::invoke_result_t<Fn>> {
  try {
    if constexpr (std::is_same_v<std::invoke_result_t<Fn>, void>) {
      std::invoke(std::forward<Fn>(fn));
      return {};
    } else {
      return std::invoke(std::forward<Fn>(fn));
    }
  } catch (const std::exception& e) {
    return std::unexpected(Error{1, e.what()});
  } catch (...) {
    return std::unexpected(Error{1, "unknown exception"});
  }
}

} // namespace cpp_utils
