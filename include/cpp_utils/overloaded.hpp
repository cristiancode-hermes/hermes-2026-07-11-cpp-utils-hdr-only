#pragma once

#include <variant>

namespace cpp_utils {

// ── overloaded: Pattern-matching visitor for std::variant ──
// Usage:
//   std::visit(overloaded{
//     [](int i) { ... },
//     [](std::string s) { ... },
//   }, my_variant);
template <typename... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

// CTAD deduction guide (C++17+)
template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace cpp_utils
