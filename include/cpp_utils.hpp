#pragma once

// ────────────────────────────────────────────────────────
// cpp_utils — Header-only C++20 utility library
// Single-include convenience header
// ────────────────────────────────────────────────────────

#include <cpp_utils/overloaded.hpp>
#include <cpp_utils/scope_guard.hpp>
#include <cpp_utils/enum_flags.hpp>
#include <cpp_utils/str.hpp>
#include <cpp_utils/expected_ext.hpp>
#include <cpp_utils/traits.hpp>

// Re-export everything into the cpp_utils namespace for convenience
namespace cpp_utils {
  using namespace cpp_utils::detail;  // scope_guard, scope_fail, scope_success
  // enum_flags, str, expected_ext, traits are already in cpp_utils
} // namespace cpp_utils
