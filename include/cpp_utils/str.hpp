#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <cctype>
#include <concepts>
#include <ranges>
#include <span>
#include <charconv>
#include <expected>
#include <system_error>
#include <format>
#include <sstream>

namespace cpp_utils {

// ── String utility functions (C++20, ranges-heavy) ──

namespace str {

// ── Split string by delimiter ──
// Returns vector of string_views (no allocations per token)
inline std::vector<std::string_view> split(std::string_view s, char delim) {
  std::vector<std::string_view> result;
  auto range = s | std::views::split(delim);
  for (auto part : range) {
    result.emplace_back(std::string_view(part.begin(), part.size()));
  }
  return result;
}

// ── Split string by delimiter string ──
inline std::vector<std::string_view> split(std::string_view s, std::string_view delim) {
  std::vector<std::string_view> result;
  size_t start = 0;
  size_t end;
  while ((end = s.find(delim, start)) != std::string_view::npos) {
    result.push_back(s.substr(start, end - start));
    start = end + delim.size();
  }
  result.push_back(s.substr(start));
  return result;
}

// ── Split into N parts max ──
inline std::vector<std::string_view> split_n(std::string_view s, char delim, size_t max_parts) {
  std::vector<std::string_view> result;
  size_t start = 0;
  while (result.size() + 1 < max_parts) {
    auto pos = s.find(delim, start);
    if (pos == std::string_view::npos) break;
    result.push_back(s.substr(start, pos - start));
    start = pos + 1;
  }
  result.push_back(s.substr(start));
  return result;
}

// ── Join strings ──
inline std::string join(std::span<const std::string> parts, std::string_view delim) {
  if (parts.empty()) return {};
  std::string result = parts[0];
  for (size_t i = 1; i < parts.size(); ++i) {
    result.append(delim);
    result.append(parts[i]);
  }
  return result;
}

inline std::string join(std::span<const std::string_view> parts, std::string_view delim) {
  if (parts.empty()) return {};
  std::string result(parts[0]);
  for (size_t i = 1; i < parts.size(); ++i) {
    result.append(delim);
    result.append(parts[i]);
  }
  return result;
}

// ── Trim whitespace (lvalue) ──
inline std::string_view trim_left(std::string_view s) noexcept {
  auto it = std::ranges::find_if_not(s, [](unsigned char c) { return std::isspace(c); });
  s.remove_prefix(static_cast<size_t>(it - s.begin()));
  return s;
}

inline std::string_view trim_right(std::string_view s) noexcept {
  auto it = std::ranges::find_if_not(s | std::views::reverse, [](unsigned char c) { return std::isspace(c); });
  s.remove_suffix(static_cast<size_t>(it - s.rbegin()));
  return s;
}

inline std::string_view trim(std::string_view s) noexcept {
  return trim_left(trim_right(s));
}

// ── Case conversion ──
inline std::string to_lower(std::string_view s) {
  std::string result(s.size(), '\0');
  std::ranges::transform(s, result.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return result;
}

inline std::string to_upper(std::string_view s) {
  std::string result(s.size(), '\0');
  std::ranges::transform(s, result.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
  return result;
}

// ── Case-insensitive comparison ──
inline bool iequals(std::string_view a, std::string_view b) noexcept {
  return std::ranges::equal(a, b, [](unsigned char ca, unsigned char cb) {
    return std::tolower(ca) == std::tolower(cb);
  });
}

// ── Replace all occurrences ──
inline std::string replace_all(std::string_view s, std::string_view from, std::string_view to) {
  std::string result;
  size_t pos = 0;
  size_t last = 0;
  while ((pos = s.find(from, last)) != std::string_view::npos) {
    result.append(s.substr(last, pos - last));
    result.append(to);
    last = pos + from.size();
  }
  result.append(s.substr(last));
  return result;
}

// ── Starts with / Ends with (C++20 already has these in string_view) ──
// using std::string_view::starts_with, ::ends_with directly

// ── Safe numeric conversion ──
template <std::integral T>
inline std::expected<T, std::errc> to_int(std::string_view s, int base = 10) {
  T value{};
  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value, base);
  if (ec != std::errc{}) return std::unexpected(ec);
  return value;
}

template <std::floating_point T>
inline std::expected<T, std::errc> to_float(std::string_view s) {
  T value{};
  auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
  if (ec != std::errc{}) return std::unexpected(ec);
  return value;
}

// ── Format with compile-time format string (C++20 std::format) ──
template <typename... Args>
inline std::string fmt(std::format_string<Args...> fmt_str, Args&&... args) {
  return std::format(fmt_str, std::forward<Args>(args)...);
}

// ── Check if string contains any of the given substrings ──
inline bool contains_any(std::string_view s, std::span<const std::string_view> needles) {
  return std::ranges::any_of(needles, [s](auto&& n) { return s.find(n) != std::string_view::npos; });
}

// ── Check if string contains all of the given substrings ──
inline bool contains_all(std::string_view s, std::span<const std::string_view> needles) {
  return std::ranges::all_of(needles, [s](auto&& n) { return s.find(n) != std::string_view::npos; });
}

} // namespace str
} // namespace cpp_utils
